/*-****************************************************************************

 * Copyright (C) 2012-2019 André Guerreiro - <aguerreiro1985@gmail.com>
 * Copyright (C) 2013 Vasco Dias - <vasco.dias@caixamagica.pt>
 * Copyright (C) 2016-2017 Luiz Lemos - <luiz.lemos@caixamagica.pt>
 * Copyright (C) 2017-2021 Adriano Campos - <adrianoribeirocampos@gmail.com>
 * Copyright (C) 2018-2019 Veniamin Craciun - <veniamin.craciun@caixamagica.pt>
 * Copyright (C) 2019 Miguel Figueira - <miguel.figueira@caixamagica.pt>
 *
 * Licensed under the EUPL V.1.2

****************************************************************************-*/

#include "poppler/Error.h"
#include "poppler/PDFDoc.h"
#include "poppler/Page.h"
#include "poppler/ErrorCodes.h"

#include "PDFSignature.h"
#include "MWException.h"
#include "eidErrors.h"
#include "MiscUtil.h"
#include "CardPteidDef.h"
#include "Log.h"
#include "Util.h"
#include "APLConfig.h"
#include "cryptoFwkPteid.h"

#include <string>

//For the setSSO calls
#include "CardLayer.h"
#include "goo/GooString.h"

#include <openssl/bio.h>
#include <openssl/x509.h>
#include "sign-pkcs7.h"
#include "TsaClient.h"

namespace eIDMW
{


	PDFDoc * makePDFDoc(const char *utf8Filepath) {
#ifdef WIN32
		std::string utf8Filename(utf8Filepath);
		std::wstring utf16Filename = utilStringWiden(utf8Filename);

		return new PDFDoc((wchar_t *)utf16Filename.c_str(), utf16Filename.size());
#else
		return new PDFDoc(new GooString(utf8Filepath));
#endif
	}

	PDFSignature::PDFSignature()
	{
		resetMembers();
	}

	PDFSignature::PDFSignature(const char *pdf_file_path): PDFSignature()
	{

		m_pdf_file_path = pdf_file_path;
		m_doc = makePDFDoc(pdf_file_path);
	}

	void PDFSignature::setCustomImage(unsigned char *img_data, unsigned long img_length)
	{
		this->my_custom_image.img_data = (unsigned char*)malloc(img_length);
		memcpy(this->my_custom_image.img_data, img_data, img_length);
		this->my_custom_image.img_length = img_length;
	}

	void PDFSignature::setCustomSealSize(unsigned int width, unsigned int height)
	{
		m_sig_width = BIGGER(width, SEAL_MINIMUM_WIDTH);
		m_sig_height = BIGGER(height, SEAL_MINIMUM_HEIGHT);
	}

	PDFSignature::~PDFSignature()
	{

		free(m_citizen_fullname);
		free(m_civil_number);
		
		//Free the strdup'ed strings from batchAddFile
		for (int i = 0; i != m_files_to_sign.size(); i++)
			free(m_files_to_sign.at(i).first);

		if (my_custom_image.img_data != NULL) {
			free(my_custom_image.img_data);
		}

		if (m_doc != NULL)
			delete m_doc;
	}

	void PDFSignature::resetMembers() 
	{
		m_visible = false;
		m_page = 1;
		m_sector = 0;
		//These values result in an invisible signature
		location_x = -1;
		location_y = -1;
		m_civil_number = NULL;
		m_citizen_fullname = NULL;
		m_attributeSupplier = NULL;
		m_attributeName = NULL;
		m_batch_mode = false;
		m_level = LEVEL_BASIC;
		m_isLandscape = false;
		m_small_signature = false;
		my_custom_image = {};
		m_doc = NULL;
		m_card = NULL;
		m_signerInfo = NULL;
		m_pkcs7 = NULL;
		m_outputName = NULL;
		m_signStarted = false;
		m_isExternalCertificate = false;
		m_isCC = true;
	}

	void PDFSignature::setFile(const char *pdf_file_path)
	{
		m_batch_mode = false;
		m_pdf_file_path = strdup(pdf_file_path);
		m_doc = makePDFDoc(pdf_file_path);
	}

	void PDFSignature::batchAddFile(char *file_path, bool last_page)
	{
		m_batch_mode = true;
		m_files_to_sign.push_back(std::make_pair(_strdup(file_path), last_page));

	}

	void PDFSignature::enableTimestamp()
	{
		m_level = LEVEL_TIMESTAMP;
	}

	void PDFSignature::setSignatureLevel(APL_SignatureLevel level)
	{
		m_level = level;
	}

	void PDFSignature::enableSmallSignature()
	{
		m_small_signature = true;
	}


	void PDFSignature::setVisible(unsigned int page_number, int sector_number)
	{
		m_visible = true;
		m_page = page_number;
		m_sector = sector_number;
	}

	void PDFSignature::setVisibleCoordinates(unsigned int page, double coord_x, double coord_y)
	{
	   m_visible = true;
	   m_page = page;
	   location_x = coord_x;
	   location_y = coord_y;

	}

	void PDFSignature::setBatch_mode( bool batch_mode ) {
        m_batch_mode = batch_mode;
	}

	bool PDFSignature::isLandscapeFormat()
	{
		if (m_doc)
		{
			if (!m_doc->isOk())
				return false;

			Page *p = m_doc->getPage(m_page);
			PDFRectangle *p_media = p->getMediaBox();

			double height = p_media->y2, width = p_media->x2;

			//Take into account page rotation
			int page_rotate = p->getRotate();
			if (page_rotate == 90 || page_rotate == 270)
			{
				height = p_media->x2;
				width = p_media->y2;
			}

			return width > height;
		}

		return false;

	}

	/* DEPRECATED: only kept for SDK compatibility */
	PDFRectangle PDFSignature::computeSigLocationFromSectorLandscape(double page_height, double page_width, int sector)
	{
		PDFRectangle sig_rect;
		double vert_align = 16; //Add this to vertically center inside each cell
		//Number of columns for portrait layout
		int columns = 4;
		double signature_height = m_small_signature ? m_sig_height / 2.0 : m_sig_height;
		int MAX_SECTOR = m_small_signature ? 40 : 20;
		const double n_lines = MAX_SECTOR / 4.0;

		double sig_width = (page_width - lr_margin*2) / columns;

		//Add left margin
		sig_rect.x1 = lr_margin;
		sig_rect.x2 = lr_margin;

		if (sector < 1 || sector > MAX_SECTOR)
			MWLOG(LEV_ERROR, MOD_APL, "Illegal value for signature page sector: %u Valid values [1-%d]",
					sector, MAX_SECTOR);

		if (sector < MAX_SECTOR - 3)
		{
			int line = sector / 4 + 1;
			if (sector % 4 == 0)
			   line = sector / 4;

			sig_rect.y1 += (page_height - 2*m_tb_margin) * (n_lines-line) / n_lines;
			sig_rect.y2 += (page_height - 2*m_tb_margin) * (n_lines-line) / n_lines;
		}

		int column = (sector-1) % 4;

		sig_rect.x1 += (column * sig_width);
		sig_rect.x2 += (column * sig_width);

		sig_rect.y1 += m_tb_margin + vert_align;

		//Define height and width of the rectangle
		sig_rect.x2 += sig_width;
		sig_rect.y2 += signature_height + m_tb_margin + vert_align;


		MWLOG(LEV_DEBUG, MOD_APL, "computeSigLocationFromSectorLandscape: Sector: %02d Location = (%f, %f) (%f, %f) \n", sector, sig_rect.x1, sig_rect.y1, sig_rect.x2, sig_rect.y2);

		return sig_rect;
	}

	/* DEPRECATED: only kept for SDK compatibility */
	PDFRectangle PDFSignature::computeSigLocationFromSector(double page_height, double page_width, int sector)
	{
		MWLOG(LEV_DEBUG, MOD_APL, "computeSigLocationFromSector called with sector=%d and m_small_signature = %d", sector, m_small_signature);

		int MAX_SECTOR = m_small_signature ? 36 : 18;
		double signature_height = m_small_signature ? m_sig_height / 2.0 : m_sig_height;
		const double n_lines = MAX_SECTOR / 3.0;
		// Add padding, adjust to subtly tweak the location
		// The units for x_pad, y_pad, sig_height and sig_width are postscript
		// points (1 px == 0.75 points)
		PDFRectangle sig_rect;
		double vert_align = 16; //Add this to vertically center inside each cell
		//Number of columns for portrait layout
		int columns = 3;

		double sig_width = (page_width - lr_margin*2) / columns;

		//Add left margin
		sig_rect.x1 = lr_margin;
		sig_rect.x2 = lr_margin;

		if (sector < 1 || sector > MAX_SECTOR)
			MWLOG(LEV_ERROR, MOD_APL, "Illegal value for signature page sector: %u Valid values [1-%d]",
					sector, MAX_SECTOR);

		if (sector < MAX_SECTOR -2)
		{
			int line = sector / 3 + 1;
			if (sector % 3 == 0)
			   line = sector / 3;

			sig_rect.y1 += (page_height - 2*m_tb_margin) * (n_lines-line) / n_lines;
			sig_rect.y2 += (page_height - 2*m_tb_margin) * (n_lines-line) / n_lines;
		}

		if (sector % 3 == 2 )
		{
			sig_rect.x1 += sig_width;
			sig_rect.x2 += sig_width;
		}

		if (sector % 3 == 0 )
		{
			sig_rect.x1 += sig_width * 2.0;
			sig_rect.x2 += sig_width * 2.0;
		}

		sig_rect.y1 += m_tb_margin + vert_align;

		//Define height and width of the rectangle
		sig_rect.x2 += sig_width;
		sig_rect.y2 += signature_height + m_tb_margin + vert_align;

		MWLOG(LEV_DEBUG, MOD_APL, "computeSigLocationFromSector: Sector: %02d Location = (%f, %f) (%f, %f) \n", sector, sig_rect.x1, sig_rect.y1, sig_rect.x2, sig_rect.y2);
		return sig_rect;
	}

	CByteArray PDFSignature::getCitizenCertificate()
	{
		MWLOG(LEV_DEBUG, MOD_APL, "DEBUG: getCitizenCertificate() This should only be called for Card Signature!");

		CByteArray certData;
		m_card->readFile(PTEID_FILE_CERT_SIGNATURE, certData);

		return certData;
	}

	void PDFSignature::setSCAPAttributes(const char * citizenName, const char * citizenId,
	                      const char * attributeSupplier, const char * attributeName) {

		m_attributeSupplier = attributeSupplier;
		m_attributeName = attributeName;
		m_citizen_fullname = (char *) citizenName;
		m_civil_number = (char *)citizenId;

	}

	void PDFSignature::parseCitizenDataFromCert(CByteArray &certData) {
		const int cert_field_size = 256;

		unsigned char * cert_data = certData.GetBytes();
		char *data_serial = (char *)malloc(cert_field_size);
		char *data_common_name = (char *)malloc(cert_field_size);

		X509 *x509 = d2i_X509(NULL, (const unsigned char**)&cert_data, certData.Size());

		if (x509 == NULL) {
			MWLOG(LEV_ERROR, MOD_APL, L"parseCitizenDataFromCert() Error decoding certificate data!");
			free(data_serial);
			free(data_common_name);
			return;
		}

		X509_NAME * subj = X509_get_subject_name(x509);
		X509_NAME_get_text_by_NID(subj, NID_serialNumber, data_serial, cert_field_size);
		X509_NAME_get_text_by_NID(subj, NID_commonName, data_common_name, cert_field_size);

		m_civil_number = data_serial;
		m_citizen_fullname = data_common_name;
		X509_free(x509);
	}

	int PDFSignature::getPageCount()  {
		if (!m_doc->isOk()) {
			fprintf(stderr,"getPageCount(): Probably broken PDF...\n");
			return -1;
		}
		if (m_doc->isEncrypted()) {
			fprintf(stderr, "getPageCount(): Encrypted PDFs are unsupported at the moment\n");
			return -2;
		}
		if (m_doc->containsXfaForm()) {
			return -3;
		}
		return m_doc->getNumPages();
	}

	char* PDFSignature::getOccupiedSectors(int page)
	{
		if (m_doc)
			return m_doc->getOccupiedSectors(page);
		else
			return "";
	}

	#ifdef WIN32
	#define PATH_SEP "\\"
	#else
	#define PATH_SEP "/"
	#endif
	std::string PDFSignature::generateFinalPath(const char *output_dir, const char *path)
	{
		char * pdf_filename = Basename((char*)path);
		std::string clean_filename = CPathUtil::remove_ext_from_basename(pdf_filename);

		int equal_filename_count = 0;
		for (unsigned int i = 0; i < unique_filenames.size(); i++)
		{
			std::string current_file_name = unique_filenames.at(i).first;
			if (clean_filename.compare(current_file_name) == 0) {
				//unique_filenames contains clean_filename
				equal_filename_count = ++unique_filenames.at(i).second;
				break;
			}
		}

		if (equal_filename_count == 0) {
			//clean_filename is not part of the vector, make sure it's added to it
			unique_filenames.push_back(std::make_pair(clean_filename, equal_filename_count));
		}

		std::string final_path = string(output_dir) + PATH_SEP + clean_filename;

		if(equal_filename_count > 0){
			final_path += "_" + std::to_string(equal_filename_count);
		}

		final_path += "_signed.pdf";

		return final_path;
	}

	std::string PDFSignature::getDocName() {
		std::string pdf_filename = Basename((char *)m_pdf_file_path.c_str());

		return pdf_filename;
	}

	size_t PDFSignature::getCurrentBatchSize() {
		return m_files_to_sign.size();
	}

	PDFSignature *PDFSignature::getSpecialCopy(size_t batch_index) {

		if (!m_batch_mode || m_files_to_sign.empty())
			return NULL;

		PDFSignature * my_clone = new PDFSignature();
		char * input_file = m_files_to_sign.at(batch_index).first;
		//Copy all the original signature members other than m_files_to_sign
		my_clone->setSignatureLevel(m_level);
		if (m_visible) {
			//Apply last-page flag
			int current_file_page = m_files_to_sign.at(batch_index).second ? getOtherPageCount(input_file) : m_page;
			
			my_clone->setVisibleCoordinates(current_file_page, location_x, location_y);
			if (m_small_signature) {
				my_clone->enableSmallSignature();
			}
		}
		//TODO: Using deep-copy of the image data is not ideal...
		if (my_custom_image.img_data != NULL)
			my_clone->setCustomImage(my_custom_image.img_data, my_custom_image.img_length);
		my_clone->setFile(input_file);

		return my_clone;
	}

	int PDFSignature::signFiles(const char *location,
		const char *reason, const char *outfile_path, bool isCardSign)
	{
		int rc = 0;

		if (!m_batch_mode)
		{
            rc = signSingleFile(location, reason, outfile_path, isCardSign);
		}
		//PIN-Caching is ON after the first signature
		else
		{
			 bool throwTimestampError = false;
			 bool throwLTVError = false;
			 bool cachedPin = false;
			 for (unsigned int i = 0; i < m_files_to_sign.size(); i++)
			 {
				try
				{
				 char *current_file = m_files_to_sign.at(i).first;
				 std::string f = generateFinalPath(outfile_path,
						 current_file);

				 m_doc = makePDFDoc(current_file);
				 if (!m_doc->isOk()) {
					 int error_code = m_doc->getErrorCode();
					 m_card->getCalReader()->setSSO(false);
					 MWLOG(LEV_ERROR, MOD_APL, "%s in batch mode signature! File index: %d",
						   error_code == errOpenFile ? "Failed to open file": "Invalid PDF document", i);
					 throw CBatchSignFailedException(error_code == errOpenFile ? EIDMW_FILE_NOT_OPENED: EIDMW_PDF_INVALID_ERROR, i);
				 }
				 //Set page as the last for each PDF doc
				 if (m_files_to_sign.at(i).second) {
					 m_page = m_doc->getNumPages();
				 }

				 rc += signSingleFile(location, reason, f.c_str(), isCardSign);

				 // Enable PIN cache
				 if (!cachedPin) {
					cachedPin = true;
					m_card->getCalReader()->setSSO(true);
				 }
				}
				catch (CMWException e)
				{
					if (e.GetError() != EIDMW_TIMESTAMP_ERROR && e.GetError() != EIDMW_LTV_ERROR){
						m_card->getCalReader()->setSSO(false);
						throw CBatchSignFailedException(e.GetError(), i);
					}

					// Enable PIN cache
					if (!cachedPin) {
						cachedPin = true;
						m_card->getCalReader()->setSSO(true);
					}
					m_level = LEVEL_BASIC; // Disable timestamp for the next files

					if (e.GetError() == EIDMW_TIMESTAMP_ERROR)
						throwTimestampError = true;
					else
						throwLTVError = true;
				}
			 }
			 m_card->getCalReader()->setSSO(false);

			if (throwLTVError)
				throw CMWEXCEPTION(EIDMW_LTV_ERROR);

			if (throwTimestampError)
				throw CMWEXCEPTION(EIDMW_TIMESTAMP_ERROR);
		}

		return rc;

	}

	int PDFSignature::getOtherPageCount(const char *input_path)
	{
		GooString filename(input_path);
		PDFDoc * doc = makePDFDoc(input_path);

		if (doc->getErrorCode() == errEncrypted)
		{
			MWLOG(LEV_ERROR, MOD_APL, "getOtherPageCount(): Encrypted PDFs are unsupported at the moment");
		    return -2;
		}
		if (!doc->isOk())
		{
			MWLOG(LEV_ERROR, MOD_APL, "getOtherPageCount(): Probably broken PDF...");
			return -1;
		}

		int pages = doc->getNumPages();
		delete doc;

		return pages;
	}


	int PDFSignature::signSingleFile(const char *location,
        const char *reason, const char *outfile_path, bool isCardSign)
	{

		bool isLangPT = false;
		bool showNIC = false;
		bool showDate = false;
		PDFDoc *doc = m_doc;
		//The class ctor initializes it to (0,0,0,0)
		//so we can use this for invisible sig
		PDFRectangle sig_location;

		APL_Config config_language(CConfig::EIDMW_CONFIG_PARAM_GENERAL_LANGUAGE);

		//Default value of language config is PT
		if (wcscmp(config_language.getWString(),
			CConfig::EIDMW_CONFIG_PARAM_GENERAL_LANGUAGE.csDefault) == 0) {
			isLangPT = true;
		}

		GooString *outputName;
		outputName = new GooString(outfile_path);

		if (!doc->isOk()) {
			MWLOG(LEV_ERROR, MOD_APL, "Error loading PDF document. Malformed or corrupted PDF file!");
			delete outputName;
			throw CMWEXCEPTION(EIDMW_PDF_INVALID_ERROR);
		}

		if (doc->isEncrypted())
		{
			MWLOG(LEV_ERROR, MOD_APL, "Trying to sign encrypted PDF: not supported yet.");
			delete outputName;
			throw CMWEXCEPTION(EIDMW_PDF_UNSUPPORTED_ERROR);
		}
	
		if (m_page > (unsigned int)doc->getNumPages())
		{
			MWLOG(LEV_ERROR, MOD_APL, "Signature Page %u is out of bounds for PDF file", m_page);
			throw CMWEXCEPTION(EIDMW_PDF_INVALID_PAGE_ERROR);
		}

		// A little ugly hack:
		//Force the parsing of the Page Tree structure to get the pagerefs loaded in Catalog object
		Page *p = doc->getPage(m_page);

		if (p == NULL)
		{
			MWLOG(LEV_ERROR, MOD_APL, "Failed to get page from PDFDoc object!");
			throw CMWEXCEPTION(EIDMW_PDF_INVALID_PAGE_ERROR);
		}

		if (m_visible)
		{
			//By the spec, the visible/writable area can be cropped by the CropBox, BleedBox, etc...
			//We're assuming the most common case of MediaBox matching the visible area
			PDFRectangle *p_media = p->getMediaBox();

			double height = p_media->y2, width = p_media->x2;
			m_isLandscape = isLandscapeFormat();

			//Fix dimensions for the /Rotate case
			if (p->getRotate() == 90 || p->getRotate() == 270)
			{
				double dim1 = height;
				height = width;
				width = dim1;
			}

			APL_Config config_seal(CConfig::EIDMW_CONFIG_PARAM_GUITOOL_SIGNSEALOPTIONS);

			showNIC = config_seal.getLong() & 1;
			showDate = config_seal.getLong() & 2;

			MWLOG(LEV_DEBUG, MOD_APL, L"PDFSignature: Visible signature selected. Page mediaBox: (H: %f W:%f)"
				" location_x: %f, location_y: %f, showNIC: %d, showDate: %d",
				height, width, location_x, location_y, showNIC, showDate);

			//Sig Location by sector
			if (location_x == -1)
			{
				if (m_isLandscape)
					sig_location = computeSigLocationFromSectorLandscape(height, width, m_sector);
				else
					sig_location = computeSigLocationFromSector(height, width, m_sector);
			}
			else
			{
				//"Round down" to a legal value to make sure we don't get partially off-page signatures
				if (location_y > 1.0)
					location_y = 1.0;
				if (location_x > 1.0)
					location_x = 1.0;

			    double sig_width = m_sig_width;
			    double actual_sig_height =  m_small_signature ? m_sig_height / 2.0 : m_sig_height;
				
     			// Visible signature never will be smaller than page
		 		// Except if the page is smaller then minimum size
				if (sig_width > width && width >= SEAL_MINIMUM_WIDTH)
					sig_width = width;

				if (actual_sig_height > height && height >= SEAL_MINIMUM_HEIGHT)
					actual_sig_height = height;

			    //sig_location.x1 = lr_margin+ (width-lr_margin*2)*location_x;
			    sig_location.x1 = (width)*location_x;

			    //Coordinates from the GUI are inverted => (1- location_y)
			    sig_location.y1 = (height) *(1.0-location_y);

			    sig_location.x2 = sig_location.x1 + sig_width;
			    sig_location.y2 = sig_location.y1 + actual_sig_height;

				// When batch signing with some PDFs with rotated pages, the max location_x and location_y percentages in horizontal
				// and vertical pages are different. This may cause the visible signature to be off-page. If this happens,
				// translate the signature back to the page.
				if (sig_location.x2 > width) {
					sig_location.x1 = width-sig_width;
					sig_location.x2 = width;
				}
				if (sig_location.y2 > height) {
					sig_location.y1 = height-actual_sig_height;
					sig_location.y2 = height;
				}
			}

			MWLOG(LEV_DEBUG, MOD_APL, "PDFSignature: Signature rectangle before rotation (if needed) (%f, %f, %f, %f)", sig_location.x1, sig_location.y1, sig_location.x2, sig_location.y2);

			if (p->getRotate() == 90)
			{
				//Apply Rotation of R: R' = [-y2, x1, -y1, x2]
				sig_location = PDFRectangle(height-sig_location.y2, sig_location.x1,
							height-sig_location.y1, sig_location.x2);
				MWLOG(LEV_DEBUG, MOD_APL, "PDFSignature: Rotating rectangle to (%f, %f, %f, %f)", sig_location.x1, sig_location.y1, sig_location.x2, sig_location.y2);
			} else if (p->getRotate() == 270)
			{
				//Apply Rotation of R: R' = [y1, -x2, y2, -x1]
				sig_location = PDFRectangle(sig_location.y1, width-sig_location.x2,
							sig_location.y2, width-sig_location.x1);
				MWLOG(LEV_DEBUG, MOD_APL, "PDFSignature: Rotating rectangle to (%f, %f, %f, %f)", sig_location.x1, sig_location.y1, sig_location.x2, sig_location.y2);
			}  else if (p->getRotate() == 180)
			{
				//Apply Rotation of R: R' = []
				sig_location = PDFRectangle(width-sig_location.x2, height-sig_location.y2,
							width-sig_location.x1, height-sig_location.y1);
				MWLOG(LEV_DEBUG, MOD_APL, "PDFSignature: Rotating rectangle to (%f, %f, %f, %f)", sig_location.x1, sig_location.y1, sig_location.x2, sig_location.y2);
			}
		}

		unsigned char *to_sign;

		if (m_isExternalCertificate && m_attributeSupplier == NULL) {
			parseCitizenDataFromCert(m_externCertificate);
		}
		else {
			//Civil number and name should be only read once
			if (m_civil_number == NULL) {
				m_certificate = getCitizenCertificate();
				parseCitizenDataFromCert(m_certificate);

				/*
				Get the certificate length to trim the padding zero bytes which are useless
				and affect the certificate digest computation
				*/
				long certLen = der_certificate_length(m_certificate);
				m_certificate = m_certificate.GetBytes(0, certLen);
		   	}
		}

		if (m_attributeSupplier != NULL) {
			doc->addSCAPAttributes(m_attributeSupplier, m_attributeName);
		}

		m_incrementalMode = doc->isSigned() || doc->isReaderEnabled();

		if (this->my_custom_image.img_data != NULL)
			doc->addCustomSignatureImage(my_custom_image.img_data, my_custom_image.img_length);

		if(showNIC)
		{
			// remove "BI" prefix and checkdigit from NIC
			std::string nic = m_civil_number;
			size_t offset = 0;
			size_t nic_length = 8;
			if (nic.find("BI") == 0){
				offset = 2;
			}
			nic = nic.substr(offset, nic_length);
			doc->prepareSignature(m_incrementalMode, &sig_location, m_citizen_fullname, nic.c_str(),
	                                 location, reason, m_page, m_sector, isLangPT, isCC(), showDate, m_small_signature);
		} else {
			doc->prepareSignature(m_incrementalMode, &sig_location, m_citizen_fullname, NULL,
	                                 location, reason, m_page, m_sector, isLangPT, isCC(), showDate, m_small_signature);
		}

		unsigned long len = doc->getSigByteArray(&to_sign, m_incrementalMode);

		int rc = 0;
		try
		{
			m_outputName = outputName;

			if (m_isExternalCertificate) {
				m_certificate = m_externCertificate;
			}

			computeHash(to_sign, len, m_certificate, m_ca_certificates, isCardSign);

			m_signStarted = true;
		}
		catch(CMWException e)
		{
			//Throw away the changed PDFDoc object because we might retry the signature
			//and this document is "half-signed"...
			if (e.GetError() == EIDMW_ERR_CARD_RESET)
			{
				delete m_doc;
				if (!m_batch_mode)
					m_doc = makePDFDoc(m_pdf_file_path.c_str());
			}
			throw;
		}

		if (to_sign) free(to_sign);

		if ( !m_isExternalCertificate) {

			/* Get card signature from card */
			CByteArray signature = PteidSign( m_card, m_hash );
			rc = signClose( signature );
		}

		return rc;
	}

    bool PDFSignature::isCC(){
		return m_isCC;
    }

    void PDFSignature::setIsCC(bool in_IsCC){
		m_isCC = in_IsCC;
    }

    void PDFSignature::setExternCertificate( CByteArray certificate) {
		m_externCertificate = certificate;

		m_isExternalCertificate = true;
    }

    void PDFSignature::setExternCertificateCA(std::vector<CByteArray> &certificateCAS) {
		m_ca_certificates = certificateCAS;
    }

    /* Hash */
    CByteArray PDFSignature::getHash() {
        return m_hash;
    }

    void PDFSignature::setHash(CByteArray in_hash) {
        m_hash = in_hash;
    }

    void PDFSignature::computeHash(unsigned char *data, unsigned long dataLen,
                                     CByteArray certificate,
                                     std::vector<CByteArray> &certificate_cas,
                                     bool isCardSign) {

        OpenSSL_add_all_algorithms();
        if ( m_pkcs7 != NULL ) PKCS7_free( m_pkcs7 );

        /* Calculate hash */
        m_pkcs7 = PKCS7_new();

        bool timestamp = (m_level == LEVEL_TIMESTAMP
                        || m_level == LEVEL_LT
                        || m_level == LEVEL_LTV);

        CByteArray in_hash = computeHash_pkcs7( data, dataLen
                                                , certificate
                                                , certificate_cas
                                                , timestamp
                                                , m_pkcs7
                                                , &m_signerInfo
                                                , isCardSign ? m_card : NULL);
        setHash(in_hash);
    }

    int PDFSignature::signClose(CByteArray signature) {

        if (!m_signStarted) {
            MWLOG(LEV_DEBUG, MOD_APL, "signClose: Signature not started");
            return -1;
        }

        const char *signature_contents = NULL;

        if (NULL == m_doc) {
            fprintf(stderr, "NULL m_doc\n");

            if (m_pkcs7 != NULL)
                PKCS7_free(m_pkcs7);
            throw CMWEXCEPTION(EIDMW_ERR_UNKNOWN);
        }

        bool timestamp = (m_level == LEVEL_TIMESTAMP || m_level == LEVEL_LT || m_level == LEVEL_LTV);

        int return_code =
            getSignedData_pkcs7((unsigned char*)signature.GetBytes()
            , signature.Size()
            , m_signerInfo
            , timestamp
            , m_pkcs7
            , &signature_contents);

        //Don't report "unknown error" exception in the case of timestamp error
        if (return_code > 1)
            throw CMWEXCEPTION(EIDMW_ERR_UNKNOWN);

        m_doc->closeSignature(signature_contents);
        save();

        if (m_level == LEVEL_LT || m_level == LEVEL_LTV) {
            if(!addLtv()) {
                return_code = 2;
            }
        }

        m_signStarted = false;

        free((void *)signature_contents);

        delete m_outputName;
        m_outputName = NULL;

        delete m_doc;
        m_doc = NULL;

        PKCS7_free( m_pkcs7 );
        m_pkcs7 = NULL;

        MWLOG(LEV_DEBUG, MOD_APL, "PDFSignature::signClose return_code = %d", return_code);

        if (return_code == 1) {
            throw CMWEXCEPTION(EIDMW_TIMESTAMP_ERROR);
        }
        else if (return_code == 2)
        {
            throw CMWEXCEPTION(EIDMW_LTV_ERROR);
        }
        return return_code;
    }

    void PDFSignature::save() {
        PDFWriteMode pdfWriteMode =
            m_incrementalMode ? writeForceIncremental : writeForceRewrite;
        // Create and save pdf to temp file to allow overwrite of original file
#ifdef WIN32
        TCHAR tmpPathBuffer[MAX_PATH];
        DWORD lengthCopied = GetTempPath(MAX_PATH, tmpPathBuffer);
        if (lengthCopied > MAX_PATH)
        {
            MWLOG(LEV_ERROR, MOD_APL, "signClose: tmpPath does not fit in buffer");
            throw CMWEXCEPTION(EIDMW_ERR_UNKNOWN);
        }
        if (lengthCopied == 0)
        {
            MWLOG(LEV_ERROR, MOD_APL, "signClose: Error occurred getting tmpPath: %d", GetLastError());
            throw CMWEXCEPTION(EIDMW_ERR_UNKNOWN);
        }
        TCHAR tmpFilename[MAX_PATH];
        if (!GetTempFileName(tmpPathBuffer,
            TEXT("tmp"),
            0,
            tmpFilename)) {
            MWLOG(LEV_ERROR, MOD_APL, "signClose: Error occurred GetTempFileName: %d", GetLastError());
            throw CMWEXCEPTION(EIDMW_ERR_UNKNOWN);
        }
#ifdef UNICODE
        std::string utf8FilenameTmp = utilStringNarrow(tmpFilename);
#else
        std::string utf8FilenameTmp = tmpFilename;
#endif
        std::wstring utf16FilenameTmp = utilStringWiden(utf8FilenameTmp);
        int tmp_ret = m_doc->saveAs((wchar_t *)utf16FilenameTmp.c_str(), pdfWriteMode);
        PDFDoc *tmpDoc = makePDFDoc(utf8FilenameTmp.c_str());
        int final_ret = tmpDoc->saveAs((wchar_t *)utilStringWiden(m_outputName->getCString()).c_str());
#else
        char tmpFilename[L_tmpnam];
        if (!tmpnam(tmpFilename)) {
            MWLOG(LEV_ERROR, MOD_APL, "signClose: Error occurred generating tmp filename");
            throw CMWEXCEPTION(EIDMW_ERR_UNKNOWN);
        }
        std::string utf8FilenameTmp = tmpFilename;
        GooString tmpFilenameGoo(utf8FilenameTmp.c_str());
        int tmp_ret = m_doc->saveAs(&tmpFilenameGoo, pdfWriteMode);
        PDFDoc *tmpDoc = makePDFDoc(utf8FilenameTmp.c_str());
        int final_ret = tmpDoc->saveAs(m_outputName);
#endif
        delete tmpDoc;
        tmpDoc = NULL;
        remove(utf8FilenameTmp.c_str());

        delete m_doc;
        m_doc = makePDFDoc(m_outputName->getCString());

        if (tmp_ret == errPermission || tmp_ret == errOpenFile) {
            throw CMWEXCEPTION(EIDMW_PERMISSION_DENIED);
        }
        else if (tmp_ret != errNone) {
            throw CMWEXCEPTION(EIDMW_ERR_UNKNOWN);
        }

        if (final_ret == errPermission || final_ret == errOpenFile) {
            throw CMWEXCEPTION(EIDMW_PERMISSION_DENIED);
        }
        else if (final_ret != errNone) {
            throw CMWEXCEPTION(EIDMW_ERR_UNKNOWN);
        }
    }

    bool PDFSignature::addLtv()
    {
        PAdESExtender padesExtender(this);
        if (m_level == LEVEL_LT)
        {
            return padesExtender.addLT();
        }
        else if (m_level == LEVEL_LTV)
        {
            return padesExtender.addLTA();
        }
        return false;
    }
}
