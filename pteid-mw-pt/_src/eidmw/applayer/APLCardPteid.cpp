/* ****************************************************************************

 * eID Middleware Project.
 * Copyright (C) 2008-2009 FedICT.
 * Copyright (C) 2019 Caixa Magica Software.
 * Copyright (C) 2011 Vasco Silva - <vasco.silva@caixamagica.pt>
 * Copyright (C) 2011-2012 lmcm - <lmcm@caixamagica.pt>
 * Copyright (C) 2011-2012 Rui Martinho - <rui.martinho@ama.pt>
 * Copyright (C) 2012, 2016-2021 André Guerreiro - <aguerreiro1985@gmail.com>
 * Copyright (C) 2016-2017 Luiz Lemos - <luiz.lemos@caixamagica.pt>
 * Copyright (C) 2019 Veniamin Craciun - <veniamin.craciun@caixamagica.pt>
 * Copyright (C) 2019 Adriano Campos - <adrianoribeirocampos@gmail.com>
 *
 * This is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version
 * 3.0 as published by the Free Software Foundation.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this software; if not, see
 * http://www.gnu.org/licenses/.

**************************************************************************** */
#include "APLCardPteid.h"
#include "CardPteid.h"
#include "TLVBuffer.h"
#include "APLCertif.h"
#include "cryptoFwkPteid.h"
#include "CardPteidDef.h"
#include "eidErrors.h"
#include "Util.h"
#include "Log.h"
#include "MWException.h"
#include "CardLayer.h"
#include "MiscUtil.h"
#include "SSLConnection.h"
#include "SAM.h"
#include "StringOps.h"
#include "APLConfig.h"
#include "RemoteAddress.h"
#include "RemoteAddressRequest.h"

#include <ctime>

namespace eIDMW
{

/*****************************************************************************************
---------------------------------------- APL_EIDCard -----------------------------------------
*****************************************************************************************/

APL_EIDCard::APL_EIDCard(APL_ReaderContext *reader, APL_CardType cardType):APL_SmartCard(reader)
{
	m_cardType = cardType;
	m_CCcustomDoc=NULL;
	m_docid=NULL;
	m_personal=NULL;
	m_address=NULL;
	m_sod=NULL;
	m_docinfo=NULL;

	m_FileTrace=NULL;
	m_FileID=NULL;
	m_FileAddress=NULL;
	m_FileSod=NULL;
	m_FilePersoData=NULL;
	m_FileTokenInfo=NULL;

	m_cardinfosign=NULL;

	m_fileCertAuthentication=NULL;
	m_fileCertSignature=NULL;
	m_fileCertRootSign=NULL;
	m_fileCertRoot=NULL;
	m_fileCertRootAuth=NULL;

	m_sodCheck = false;
	m_tokenLabel = NULL;
	m_tokenSerial = NULL;
	m_appletVersion = NULL;
}

APL_EIDCard::~APL_EIDCard()
{
	if(m_CCcustomDoc)
	{
		delete m_CCcustomDoc;
		m_CCcustomDoc=NULL;
	}
	if(m_docid)
	{
		delete m_docid;
		m_docid=NULL;
	}
	if(m_personal)
	{
		delete m_personal;
		m_personal=NULL;
	}
	if(m_address)
	{
		delete m_address;
		m_address=NULL;
	}
	if(m_sod)
	{
		delete m_sod;
		m_sod=NULL;
	}

	if(m_docinfo)
	{
		delete m_docinfo;
		m_docinfo=NULL;
	}

	if(m_FileTrace)
	{
		delete m_FileTrace;
		m_FileTrace=NULL;
	}

	if(m_FileID)
	{
		delete m_FileID;
		m_FileID=NULL;
	}
	
	if(m_FileAddress)
	{
		delete m_FileAddress;
		m_FileAddress=NULL;
	}

	if(m_FileSod)
	{
		delete m_FileSod;
		m_FileSod=NULL;
	}

	if(m_FilePersoData)
	{
		delete m_FilePersoData;
		m_FilePersoData=NULL;
	}

	if(m_FileTokenInfo)
	{
		delete m_FileTokenInfo;
		m_FileTokenInfo=NULL;
	}

	if(m_cardinfosign)
	{
		delete m_cardinfosign;
		m_cardinfosign=NULL;
	}

	if(m_fileCertAuthentication)
	{
		delete m_fileCertAuthentication;
		m_fileCertAuthentication=NULL;
	}

	if(m_fileCertSignature)
	{
		delete m_fileCertSignature;
		m_fileCertSignature=NULL;
	}

	if(m_fileCertRootAuth)
	{
		delete m_fileCertRootAuth;
		m_fileCertRootAuth=NULL;
	}

	if(m_fileCertRootSign)
	{
		delete m_fileCertRootSign;
		m_fileCertRootSign=NULL;
	}

	if(m_fileCertRoot)
	{
		delete m_fileCertRoot;
		m_fileCertRoot=NULL;
	}

	if(m_tokenLabel)
	{
		delete m_tokenLabel;
		m_tokenLabel=NULL;
	}

	if (m_tokenSerial)
	{
		delete m_tokenSerial;
		m_tokenSerial=NULL;
	}

}

APL_EidFile_Trace *APL_EIDCard::getFileTrace()
{
		CAutoMutex autoMutex(&m_Mutex);		//We lock for only one instanciation
		m_FileTrace=new APL_EidFile_Trace(this);


	return m_FileTrace;
}

APL_EidFile_ID *APL_EIDCard::getFileID()
{
	if(!m_FileID)
	{
		CAutoMutex autoMutex(&m_Mutex);		//We lock for only one instanciation
		if(!m_FileID)
			m_FileID=new APL_EidFile_ID(this);
	}

	return m_FileID;
}

APL_EidFile_Address *APL_EIDCard::getFileAddress()
{
	if(!m_FileAddress)
	{
		CAutoMutex autoMutex(&m_Mutex);		//We lock for only one instanciation
		if(!m_FileAddress)
			m_FileAddress=new APL_EidFile_Address(this);
	}

	return m_FileAddress;
}


/*
	Implements the address change protocol as implemented by the Portuguese State hosted website
	It conditionally executes some card interactions and sends different parameters to the server
	depending on the version of the smart card applet, IAS 0.7 or IAS 1.01.
	Technical specification: the confidential document "Change Address Technical Solution" by Zetes version 5.3
*/
void APL_EIDCard::ChangeAddress(char *secret_code, char *process, t_callback_addr callback, void* callback_data)
{
	char * kicc = NULL;
	SAM sam_helper(this);
	StartWriteResponse * resp3 = NULL;
	char *serialNumber = NULL;
	char *resp_internal_auth = NULL, *resp_mse = NULL;

	DHParams dh_params;

	if (this->getType() == APL_CARDTYPE_PTEID_IAS07)
		sam_helper.getDHParams(&dh_params, true);
	else
	{
	    throw CMWEXCEPTION(EIDMW_SAM_UNSUPPORTED_CARD);
	}
	//Init the thread-local variable to be able to access the card in the OpenSSL callback
	setThreadLocalCardInstance(this);
	SSLConnection conn;

	conn.InitSAMConnection();

	callback(callback_data, 10);

	DHParamsResponse *p1 = conn.do_SAM_1stpost(&dh_params, secret_code, process, serialNumber);

	callback(callback_data, 25);

	if (p1->cv_ifd_aut == NULL)
	{
		delete p1;
		throw CMWEXCEPTION(EIDMW_SAM_PROTOCOL_ERROR);
	}

	if (p1->kifd != NULL)
		sam_helper.sendKIFD(p1->kifd);

	kicc = sam_helper.getKICC();

	if ( !sam_helper.verifyCert_CV_IFD(p1->cv_ifd_aut))
	{
		delete p1;
		free(kicc);

		throw CMWEXCEPTION(EIDMW_SAM_PROTOCOL_ERROR);
	}

	char * CHR = sam_helper.getPK_IFD_AUT(p1->cv_ifd_aut);

	char *challenge = sam_helper.generateChallenge(CHR);

	callback(callback_data, 30);

	SignedChallengeResponse * resp_2ndpost = conn.do_SAM_2ndpost(challenge, kicc);

	callback(callback_data, 40);

	if (resp_2ndpost != NULL && resp_2ndpost->signed_challenge != NULL)
	{
		bool ret_signed_ch = sam_helper.verifySignedChallenge(resp_2ndpost->signed_challenge);

		if (!ret_signed_ch)
		{
            delete resp_2ndpost;
            free(challenge);
            free(CHR);
			MWLOG(LEV_ERROR, MOD_APL, L"EXTERNAL AUTHENTICATE command failed! Aborting Address Change!");
			throw CMWEXCEPTION(EIDMW_SAM_PROTOCOL_ERROR);
		}

		resp_mse = sam_helper.sendPrebuiltAPDU(resp_2ndpost->set_se_command);

		resp_internal_auth = sam_helper.sendPrebuiltAPDU(resp_2ndpost->internal_auth);

		StartWriteResponse * resp3 = conn.do_SAM_3rdpost(resp_mse, resp_internal_auth);

		callback(callback_data, 60);

		if (resp3 == NULL)
		{
			goto err;
		}
		else
		{
			MWLOG(LEV_DEBUG, MOD_APL, L"ChangeAddress: writing new address...");
			std::vector<char *> address_response = sam_helper.sendSequenceOfPrebuiltAPDUs(resp3->apdu_write_address);

			MWLOG(LEV_DEBUG, MOD_APL, L"ChangeAddress: writing new SOD...");
			std::vector<char *> sod_response = sam_helper.sendSequenceOfPrebuiltAPDUs(resp3->apdu_write_sod);

			StartWriteResponse start_write_resp = {address_response, sod_response};

			callback(callback_data, 90);

			// Report the results to the server for verification purposes,
			// We only consider the Address Change successful if the server returns its "Acknowledge" message
			if (!conn.do_SAM_4thpost(start_write_resp)) {
                delete resp3;
                free(resp_mse);
                free(resp_internal_auth);
                MWLOG(LEV_ERROR, MOD_APL, 
                	"The Address Change process WAS ABORTED after successful card write because of unexpected server reply!");
				throw CMWEXCEPTION(EIDMW_SAM_UNCONFIRMED_CHANGE);
			}

			callback(callback_data, 100);
			invalidateAddressSOD();

			delete resp3;
			delete resp_2ndpost;
			free(challenge);
			free(CHR);
			free(resp_mse);
			free(resp_internal_auth);
			return;
		}
	}

err:
	delete resp3;
	delete resp_2ndpost;
    delete p1;
    free(kicc);
	free(challenge);
	free(CHR);
	free(resp_mse);
	free(resp_internal_auth);

	throw CMWEXCEPTION(EIDMW_SAM_PROTOCOL_ERROR);
}


/* Discard the current address and SOD file objects after a successful
   Address Change process
*/
void APL_EIDCard::invalidateAddressSOD()
{
	if(m_FileSod)
	{
		delete m_FileSod;
		m_FileSod = NULL;
	}
	if (m_FileAddress)
	{
		delete m_FileAddress;
		m_FileAddress = NULL;
	}
}

APL_EidFile_Sod *APL_EIDCard::getFileSod()
{
	if(!m_FileSod)
	{
		CAutoMutex autoMutex(&m_Mutex);		//We lock for only one instanciation
		if(!m_FileSod)
		{
			CByteArray ba_validityEndDate;     //Validity date field from card ID file
			m_FileSod=new APL_EidFile_Sod(this);

			//Read the validity end date directly from cardlayer to avoid recursive calls from APL_EidFile_ID::getValidityEndDate
			unsigned long bytes_read = 
				  this->readFile(PTEID_FILE_ID, ba_validityEndDate, PTEIDNG_FIELD_ID_POS_ValidityEndDate, PTEIDNG_FIELD_ID_LEN_ValidityEndDate);

			ba_validityEndDate.TrimRight('\0');
			MWLOG(LEV_DEBUG, MOD_APL, "ValidityEndDate freshly read: %s", ba_validityEndDate.GetBytes());
		}
	}
	return m_FileSod;
}

APL_EidFile_PersoData *APL_EIDCard::getFilePersoData()
{
	if(!m_FilePersoData)
	{
		CAutoMutex autoMutex(&m_Mutex);		//We lock for only one instanciation
		if(!m_FilePersoData)
		{
			m_FilePersoData=new APL_EidFile_PersoData(this);
		}
	}
	return m_FilePersoData;
}

APL_EidFile_TokenInfo *APL_EIDCard::getFileTokenInfo()
{
	if(!m_FileTokenInfo)
	{
		CAutoMutex autoMutex(&m_Mutex);		//We lock for only one instanciation
		if(!m_FileTokenInfo)
		{
			m_FileTokenInfo=new APL_EidFile_TokenInfo(this);
		}
	}
	return m_FileTokenInfo;
}

APL_CardType APL_EIDCard::getType() const
{
	return m_cardType;
}

unsigned long APL_EIDCard::readFile(const char *csPath, CByteArray &oData, unsigned long  ulOffset, unsigned long  ulMaxLength)
{

	return APL_SmartCard::readFile(csPath,oData,ulOffset,ulMaxLength);
}

unsigned long APL_EIDCard::certificateCount()
{

	try
	{
		return APL_SmartCard::certificateCount();
	}
	catch(...)
	{
	}

	if(m_certificateCount==COUNT_UNDEF)
	{
		//PKCS15 is broken
		CAutoMutex autoMutex(&m_Mutex);		//We lock for only one instanciation
		if(m_certificateCount==COUNT_UNDEF)
		{
			m_certificateCount=0;

			if(!m_fileCertAuthentication)
			{
				m_fileCertAuthentication=new APL_CardFile_Certificate(this,PTEID_FILE_CERT_AUTHENTICATION);
				//If status ok, we add the certificate to the store
				if(m_fileCertAuthentication->getStatus(false)==CARDFILESTATUS_OK)
				{
					if(NULL == (getCertificates()->addCert(m_fileCertAuthentication,APL_CERTIF_TYPE_AUTHENTICATION,true,false,m_certificateCount,NULL,NULL)))
						throw CMWEXCEPTION(EIDMW_ERR_CHECK);
					m_certificateCount++;
				}
			}

			if(!m_fileCertSignature)
			{
				m_fileCertSignature=new APL_CardFile_Certificate(this,PTEID_FILE_CERT_SIGNATURE);
				//If status ok, we add the certificate to the store
				if(m_fileCertSignature->getStatus(true)==CARDFILESTATUS_OK)
				{
					if(NULL == (getCertificates()->addCert(m_fileCertSignature,APL_CERTIF_TYPE_SIGNATURE,true,false,m_certificateCount,NULL,NULL)))
						throw CMWEXCEPTION(EIDMW_ERR_CHECK);
					m_certificateCount++;
				}
			}

			if(!m_fileCertRootAuth)
			{
				m_fileCertRootAuth=new APL_CardFile_Certificate(this,PTEID_FILE_CERT_ROOT_AUTH);
				//If status ok, we add the certificate to the store
				if(m_fileCertRootAuth->getStatus(true)==CARDFILESTATUS_OK)
				{
					if(NULL == (getCertificates()->addCert(m_fileCertRootAuth,APL_CERTIF_TYPE_ROOT_AUTH,true,false,m_certificateCount,NULL,NULL)))
						throw CMWEXCEPTION(EIDMW_ERR_CHECK);
					m_certificateCount++;
				}
			}

			if(!m_fileCertRootSign)
			{
				m_fileCertRootSign=new APL_CardFile_Certificate(this,PTEID_FILE_CERT_ROOT_SIGN);
				//If status ok, we add the certificate to the store
				if(m_fileCertRootSign->getStatus(true)==CARDFILESTATUS_OK)
				{
					if(NULL == (getCertificates()->addCert(m_fileCertRootSign,APL_CERTIF_TYPE_ROOT_SIGN,true,false,m_certificateCount,NULL,NULL)))
						throw CMWEXCEPTION(EIDMW_ERR_CHECK);
					m_certificateCount++;
				}
			}
		}
	}
	return m_certificateCount;
}


APL_CCXML_Doc& APL_EIDCard::getXmlCCDoc(APL_XmlUserRequestedInfo& userRequestedInfo){
	if (m_CCcustomDoc)
		delete m_CCcustomDoc;

	CAutoMutex autoMutex(&m_Mutex);		//We lock for only one instanciation
	m_CCcustomDoc=new APL_CCXML_Doc(this, userRequestedInfo);

	return *m_CCcustomDoc;
}


APL_DocEId& APL_EIDCard::getID()
{
	if(!m_docid)
	{
		CAutoMutex autoMutex(&m_Mutex);		//We lock for only one instanciation
		if(!m_docid)
		{
			m_docid=new APL_DocEId(this);
		}
	}

	return *m_docid;
}


APL_PersonalNotesEId& APL_EIDCard::getPersonalNotes()
{
	if(!m_personal)
		{
			CAutoMutex autoMutex(&m_Mutex);		//We lock for only one instanciation
			if(!m_personal)
			{
				m_personal=new APL_PersonalNotesEId(this);
			}
		}

		return *m_personal;
}


APL_AddrEId& APL_EIDCard::getAddr()
{
	if(!m_address)
	{
		CAutoMutex autoMutex(&m_Mutex);		//We lock for only one instanciation
		if(!m_address)
		{
			m_address=new APL_AddrEId(this);
		}
	}

	return *m_address;
}

APL_SodEid& APL_EIDCard::getSod()
{
	if(!m_sod)
	{
		CAutoMutex autoMutex(&m_Mutex);		//We lock for only one instanciation
		if(!m_sod)
		{
			m_sod=new APL_SodEid(this);
		}
	}

	return *m_sod;
}

APL_DocVersionInfo& APL_EIDCard::getDocInfo()
{
	if(!m_docinfo)
	{
		CAutoMutex autoMutex(&m_Mutex);		//We lock for only one instanciation
		if(!m_docinfo)
		{
			m_docinfo=new APL_DocVersionInfo(this);
		}
	}

	return *m_docinfo;
}

const CByteArray& APL_EIDCard::getRawData(APL_RawDataType type)
{
	switch(type)
	{
	case APL_RAWDATA_ID:
		return getRawData_Id();
	case APL_RAWDATA_TRACE:
		return getRawData_Trace();
	case APL_RAWDATA_ADDR:
		return getRawData_Addr();
	case APL_RAWDATA_SOD:
		return getRawData_Sod();
	case APL_RAWDATA_CARD_INFO:
		return getRawData_CardInfo();
	case APL_RAWDATA_TOKEN_INFO:
		return getRawData_TokenInfo();
	case APL_RAWDATA_PERSO_DATA:
		return getRawData_PersoData();
	default:
		throw CMWEXCEPTION(EIDMW_ERR_CHECK);
	}
}

const CByteArray& APL_EIDCard::getRawData_Trace()
{
	return getFileTrace()->getData();
}

const CByteArray& APL_EIDCard::getRawData_Id()
{
	return getFileID()->getData();
}

const CByteArray& APL_EIDCard::getRawData_Addr()
{
	return getFileAddress()->getData();
}

const CByteArray& APL_EIDCard::getRawData_Sod()
{
	return getFileSod()->getData();
}

const CByteArray& APL_EIDCard::getRawData_CardInfo()
{
	return getFileInfo()->getData();
}

const CByteArray& APL_EIDCard::getRawData_TokenInfo()
{
	return getFileTokenInfo()->getData();
}

const CByteArray& APL_EIDCard::getRawData_PersoData()
{
	return getFilePersoData()->getData();
}


const char *APL_EIDCard::getTokenSerialNumber(){
	if (!m_tokenSerial){

		//BEGIN_CAL_OPERATION(m_reader)
        m_reader->CalLock();
        try{
            m_tokenSerial = new string (m_reader->getCalReader()->GetSerialNr());
        } catch(...){
            m_reader->CalUnlock();
            delete m_tokenSerial;
            throw;
        }
        m_reader->CalUnlock();
		//END_CAL_OPERATION(m_reader)
	}
	return m_tokenSerial->c_str();
}

const char *APL_EIDCard::getTokenLabel(){
	if (!m_tokenLabel){

		//BEGIN_CAL_OPERATION(m_reader)
        m_reader->CalLock();
        try{
            m_tokenLabel = new string (m_reader->getCalReader()->GetCardLabel());
        } catch(...){
            m_reader->CalUnlock();
            delete m_tokenLabel;
            throw;
        }
        m_reader->CalUnlock();
		//END_CAL_OPERATION(m_reader)
	}
	return m_tokenLabel->c_str();
}

const char * APL_EIDCard::getAppletVersion() {

	if (!m_appletVersion) {
		m_reader->CalLock();
		try {
			m_appletVersion = new string(m_reader->getCalReader()->GetAppletVersion());
		}
		catch (...) {
			m_reader->CalUnlock();
			delete m_appletVersion;
			throw;
		}
		m_reader->CalUnlock();
	}
	return m_appletVersion->c_str();

}

APLPublicKey *APL_EIDCard::getRootCAPubKey(){

	if (!m_RootCAPubKey){
		CByteArray out;
		CByteArray modulus;
		CByteArray exponent;

		BEGIN_CAL_OPERATION(m_reader)
		out = m_reader->getCalReader()->RootCAPubKey();
		END_CAL_OPERATION(m_reader)


		switch(m_reader->getCardType()){
		case APL_CARDTYPE_PTEID_IAS101:
			modulus = out.GetBytes(PTEIDNG_FIELD_ROOTCA_PK_POS_MODULUS_IAS101, PTEIDNG_FIELD_ROOTCA_PK_LEN_MODULUS);
			exponent = out.GetBytes(PTEIDNG_FIELD_ROOTCA_PK_POS_EXPONENT_IAS_101, PTEIDNG_FIELD_ROOTCA_PK_LEN_EXPONENT);
			break;
		case APL_CARDTYPE_PTEID_IAS07:
			modulus = out.GetBytes(PTEIDNG_FIELD_ROOTCA_PK_POS_MODULUS_IAS07, PTEIDNG_FIELD_ROOTCA_PK_LEN_MODULUS);
			exponent = out.GetBytes(PTEIDNG_FIELD_ROOTCA_PK_POS_EXPONENT_IAS_07, PTEIDNG_FIELD_ROOTCA_PK_LEN_EXPONENT);
			break;
		case APL_CARDTYPE_UNKNOWN:
			throw CMWEXCEPTION(EIDMW_ERR_CARDTYPE_UNKNOWN);
			break;
		}

		m_RootCAPubKey = new APLPublicKey(modulus,exponent);
	}

	return m_RootCAPubKey;
}

bool APL_EIDCard::isActive()
{
	return getFileTrace()->isActive();
}

bool APL_EIDCard::Activate(const char *pinCode, CByteArray &BCDDate, bool blockActivationPIN){
	bool out = false;

	BEGIN_CAL_OPERATION(m_reader)
	out = m_reader->getCalReader()->Activate(pinCode,BCDDate, blockActivationPIN);
	END_CAL_OPERATION(m_reader)

	return out;
}

void APL_EIDCard::doSODCheck(bool check){
	m_sodCheck = check;

	if (m_FileAddress)
		m_FileAddress->doSODCheck(check);
	if (m_FileID)
		m_FileID->doSODCheck(check);

    if (!m_FileSod){
        m_FileSod = getFileSod();
        m_FileSod->doSODCheck(check);
    }
}


/*****************************************************************************************
---------------------------------------- APL_CCXML_Doc -------------------------------------------
*****************************************************************************************/
APL_CCXML_Doc::APL_CCXML_Doc(APL_EIDCard *card, APL_XmlUserRequestedInfo&  xmlUserRequestedInfo)
{
	m_card=card;
	m_xmlUserRequestedInfo = &xmlUserRequestedInfo;
}

APL_CCXML_Doc::~APL_CCXML_Doc()
{
}

CByteArray APL_CCXML_Doc::getXML(bool bNoHeader)
{
	CByteArray xml;
	string *ts, *sn, *sa, *tk;
	string rootATTRS;

	ts = m_xmlUserRequestedInfo->getTimeStamp();
	sn = m_xmlUserRequestedInfo->getServerName();
	sa = m_xmlUserRequestedInfo->getServerAddress();
	tk = m_xmlUserRequestedInfo->getTokenID();

	if(!bNoHeader)
		xml+="<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	if (ts || sn || sa || tk){
		xml+=XML_ROOT_S;
		if (ts){
			XML_ATTRIBUTE(xml,XML_ROOT_ELEMENT_ATTR_TIMESTAMP,ts->c_str());
		}
		if (sn) {
			XML_ATTRIBUTE(xml,XML_ROOT_ELEMENT_ATTR_SERVERNAME,sn->c_str());
		}
		if (sa) {
			XML_ATTRIBUTE(xml,XML_ROOT_ELEMENT_ATTR_SERVERADDRESS,sa->c_str());
		}
		if (tk) {
			XML_ATTRIBUTE(xml,XML_ROOT_ELEMENT_ATTR_TOKENID,tk->c_str());
		}
		xml+=XML_ROOT_E;
	} else
		xml+=XML_OPEN_TAG_NEWLINE(XML_ROOT_ELEMENT);

	xml+=m_card->getID().getXML(true,*m_xmlUserRequestedInfo);
	xml+=m_card->getAddr().getXML(true, *m_xmlUserRequestedInfo);
	xml+=m_card->getPersonalNotes().getXML(true, *m_xmlUserRequestedInfo);
	xml+=XML_CLOSE_TAG(XML_ROOT_ELEMENT);

	return xml;
}

CByteArray APL_CCXML_Doc::getCSV(){
	CByteArray cb;

	return cb;
}

CByteArray APL_CCXML_Doc::getTLV(){
	CByteArray cb;

	return cb;
}


/*****************************************************************************************
-------------------------------- APL_XmlUserRequestedInfo ---------------------------------------
*****************************************************************************************/
APL_XmlUserRequestedInfo::APL_XmlUserRequestedInfo()
{
	xmlSet = new set<enum XMLUserData>;
	_serverName = NULL;
	_serverAddress = NULL;
	_timeStamp = NULL;
	_tokenID = NULL;
}

APL_XmlUserRequestedInfo::APL_XmlUserRequestedInfo(const char *timeStamp, const char *serverName, const char *serverAddress)
{
	xmlSet = new set<enum XMLUserData>;
	_timeStamp = (timeStamp) ? new string(timeStamp) : NULL;
	_serverName = (serverName) ? new string(serverName) : NULL;
	_serverAddress = (serverAddress) ? new string(serverAddress) : NULL;
	_tokenID = NULL;
}

APL_XmlUserRequestedInfo::APL_XmlUserRequestedInfo(const char *timeStamp, const char *serverName, const char *serverAddress, const char *tokenID)
{
	xmlSet = new set<enum XMLUserData>;

	_timeStamp = (timeStamp) ? new string(timeStamp) : NULL;
	_serverName = (serverName) ? new string(serverName) : NULL;
	_serverAddress = (serverAddress) ? new string(serverAddress) : NULL;
	_tokenID = (tokenID) ? new string(tokenID) : NULL;
}

APL_XmlUserRequestedInfo::~APL_XmlUserRequestedInfo()
{
	if (xmlSet)
		delete xmlSet;
	if (_timeStamp)
		delete _timeStamp;
	if (_serverAddress)
		delete _serverAddress;
	if (_serverName)
		delete _serverName;
	if (_tokenID)
		delete _tokenID;
}

void APL_XmlUserRequestedInfo::add(XMLUserData xmlUData)
{
	xmlSet->insert(xmlUData);
}

bool APL_XmlUserRequestedInfo::contains(XMLUserData xmlUData)
{
	return (xmlSet->find(xmlUData) != xmlSet->end());
}

void APL_XmlUserRequestedInfo::remove(XMLUserData xmlUData)
{
	xmlSet->erase(xmlUData);
}

bool APL_XmlUserRequestedInfo::checkAndRemove(XMLUserData xmlUData)
{
	bool contains;

	contains = (xmlSet->find(xmlUData) != xmlSet->end());
	if (!contains)
		xmlSet->erase(xmlUData);

	return contains;
}

bool APL_XmlUserRequestedInfo::isEmpty(){
	return xmlSet->empty();
}

std::string* APL_XmlUserRequestedInfo::getTimeStamp(){
	return _timeStamp;
}

std::string* APL_XmlUserRequestedInfo::getServerName(){
	return _serverName;
}

std::string* APL_XmlUserRequestedInfo::getServerAddress(){
	return _serverAddress;
}

std::string* APL_XmlUserRequestedInfo::getTokenID(){
	return _tokenID;
}


/*****************************************************************************************
---------------------------------------- APL_DocEId ---------------------------------------------
*****************************************************************************************/
APL_DocEId::APL_DocEId(APL_EIDCard *card)
{
	m_card=card;

	//m_FirstName.clear();

	_xmlUInfo = NULL;
}

APL_DocEId::~APL_DocEId()
{
}


CByteArray APL_DocEId::getXML(bool bNoHeader, APL_XmlUserRequestedInfo &xmlUInfo){

	CByteArray ca;

	_xmlUInfo = &xmlUInfo;
	ca = getXML(bNoHeader);
	_xmlUInfo = NULL;

	return ca;
}

CByteArray APL_DocEId::getXML(bool bNoHeader)
{
	CByteArray xml;
	CByteArray basicInfo;
	CByteArray civilInfo;
	CByteArray idNum;
	CByteArray cardValues;
	CByteArray b64photo;
	CByteArray *photo;
	bool addBasicInfo = false;
	bool addIdNum = false;
	bool addCardValues = false;
	bool addCivilInfo = false;
	string temp;

	// photo
	if (_xmlUInfo->contains(XML_PHOTO)){
		photo = getPhotoObj()->getPhotoPNG();
		m_cryptoFwk->b64Encode(*photo,b64photo);
		BUILD_XML_ELEMENT(xml, XML_PHOTO_ELEMENT, b64photo);
	}

	// basicInformation
	if (_xmlUInfo->contains(XML_NAME)){
		string s;
		s+= getGivenName();
		s+=" ";
		s+=getSurname();
		BUILD_XML_ELEMENT(basicInfo, XML_NAME_ELEMENT, s.c_str());
		addBasicInfo = true;
	}
	if (_xmlUInfo->contains(XML_GIVEN_NAME)){
		BUILD_XML_ELEMENT(basicInfo,XML_GIVEN_NAME_ELEMENT,getGivenName());
		addBasicInfo = true;
	}
	if (_xmlUInfo->contains(XML_SURNAME)){
		BUILD_XML_ELEMENT(basicInfo,XML_SURNAME_ELEMENT,getSurname());
		addBasicInfo = true;
	}
	if (_xmlUInfo->contains(XML_NIC)){
		BUILD_XML_ELEMENT(basicInfo,XML_NIC_ELEMENT,getCivilianIdNumber());
		addBasicInfo = true;
	}
	if (_xmlUInfo->contains(XML_EXPIRY_DATE)){
		BUILD_XML_ELEMENT(basicInfo,XML_EXPIRY_DATE_ELEMENT, getValidityEndDate());
		addBasicInfo = true;
	}
	if (addBasicInfo){
		BUILD_XML_ELEMENT_NEWLINE(xml, XML_BASIC_INFO_ELEMENT, basicInfo);
	}

	// CivilInformation
	if (_xmlUInfo->contains(XML_GENDER)){
		BUILD_XML_ELEMENT(civilInfo, XML_GENDER_ELEMENT, getGender());
		addCivilInfo = true;
	}
	if (_xmlUInfo->contains(XML_HEIGHT)){
		BUILD_XML_ELEMENT(civilInfo, XML_HEIGHT_ELEMENT, getHeight());
		addCivilInfo = true;
	}
	if (_xmlUInfo->contains(XML_NATIONALITY)){
		BUILD_XML_ELEMENT(civilInfo, XML_NATIONALITY_ELEMENT, getNationality());
		addCivilInfo = true;
	}
	if (_xmlUInfo->contains(XML_DATE_OF_BIRTH)){
		BUILD_XML_ELEMENT(civilInfo, XML_DATE_OF_BIRTH_ELEMENT, getDateOfBirth());
		addCivilInfo = true;
	}
	if (_xmlUInfo->contains(XML_GIVEN_NAME_FATHER)){
		BUILD_XML_ELEMENT(civilInfo, XML_GIVEN_NAME_FATHER_ELEMENT, getGivenNameFather());
		addCivilInfo = true;
	}
	if (_xmlUInfo->contains(XML_SURNAME_FATHER)){
		BUILD_XML_ELEMENT(civilInfo, XML_SURNAME_FATHER_ELEMENT, getSurnameFather());
		addCivilInfo = true;
	}
	if (_xmlUInfo->contains(XML_GIVEN_NAME_MOTHER)){
		BUILD_XML_ELEMENT(civilInfo, XML_GIVEN_NAME_MOTHER_ELEMENT, getGivenNameMother());
		addCivilInfo = true;
	}
	if (_xmlUInfo->contains(XML_SURNAME_MOTHER)){
		BUILD_XML_ELEMENT(civilInfo, XML_SURNAME_MOTHER_ELEMENT, getSurnameMother());
		addCivilInfo = true;
	}
	if (_xmlUInfo->contains(XML_ACCIDENTAL_INDICATIONS)){
		BUILD_XML_ELEMENT(civilInfo, XML_ACCIDENTAL_INDICATIONS_ELEMENT, getAccidentalIndications());
		addCivilInfo = true;
	}
	if (addCivilInfo){
		BUILD_XML_ELEMENT_NEWLINE(xml,XML_CIVIL_INFO_ELEMENT, civilInfo);
	}

	// IdentificationNumbers
	if (_xmlUInfo->contains(XML_DOCUMENT_NO)){
		BUILD_XML_ELEMENT(idNum, XML_DOCUMENT_NO_ELEMENT, getDocumentNumber());
		addIdNum = true;
	}
	if (_xmlUInfo->contains(XML_TAX_NO)){
		BUILD_XML_ELEMENT(idNum, XML_TAX_NO_ELEMENT, getTaxNo());
		addIdNum = true;
	}
	if (_xmlUInfo->contains(XML_SOCIAL_SECURITY_NO)){
		BUILD_XML_ELEMENT(idNum, XML_SOCIAL_SECURITY_NO_ELEMENT, getSocialSecurityNumber());
		addIdNum = true;
	}
	if (_xmlUInfo->contains(XML_HEALTH_NO)){
		BUILD_XML_ELEMENT(idNum, XML_HEALTH_NO_ELEMENT, getHealthNumber());
		addIdNum = true;
	}
	if (_xmlUInfo->contains(XML_MRZ1)){
		temp = getMRZ1();
		replace(temp,XML_ESCAPE_LT);
		BUILD_XML_ELEMENT(idNum, XML_MRZ1_ELEMENT, temp);
		addIdNum = true;
	}
	if (_xmlUInfo->contains(XML_MRZ2)){
		temp = getMRZ2();
		replace(temp,XML_ESCAPE_LT);
		BUILD_XML_ELEMENT(idNum, XML_MRZ2_ELEMENT, temp);
		addIdNum = true;
	}
	if (_xmlUInfo->contains(XML_MRZ3)){
		temp = getMRZ3();
		replace(temp,XML_ESCAPE_LT);
		BUILD_XML_ELEMENT(idNum, XML_MRZ3_ELEMENT, temp);
		addIdNum = true;
	}
	if (addIdNum){
		BUILD_XML_ELEMENT_NEWLINE(xml,XML_IDENTIFICATION_NUMBERS_ELEMENT, idNum);
	}

	// CardValues
	if (_xmlUInfo->contains(XML_CARD_VERSION)){
		BUILD_XML_ELEMENT(cardValues, XML_CARD_VERSION_ELEMENT, getDocumentVersion());
		addCardValues = true;
	}
	if (_xmlUInfo->contains(XML_CARD_NUMBER_PAN)){
		BUILD_XML_ELEMENT(cardValues, XML_CARD_NUMBER_PAN_ELEMENT, getDocumentPAN());
		addCardValues = true;
	}
	if (_xmlUInfo->contains(XML_ISSUING_DATE)){
		BUILD_XML_ELEMENT(cardValues, XML_ISSUING_DATE_ELEMENT, getValidityBeginDate());
		addCardValues = true;
	}
	if (_xmlUInfo->contains(XML_ISSUING_ENTITY)){
		BUILD_XML_ELEMENT(cardValues, XML_ISSUING_ENTITY_ELEMENT, getIssuingEntity());
		addCardValues = true;
	}
	if (_xmlUInfo->contains(XML_DOCUMENT_TYPE)){
		BUILD_XML_ELEMENT(cardValues, XML_DOCUMENT_TYPE_ELEMENT, getDocumentType());
		addCardValues = true;
	}
	if (_xmlUInfo->contains(XML_LOCAL_OF_REQUEST)){
		BUILD_XML_ELEMENT(cardValues, XML_LOCAL_OF_REQUEST_ELEMENT, getLocalofRequest());
		addCardValues = true;
	}
	if (_xmlUInfo->contains(XML_VERSION)){
		BUILD_XML_ELEMENT(cardValues, XML_VERSION_ELEMENT, "0");
		addCardValues = true;
	}
	if (addCardValues){
		BUILD_XML_ELEMENT_NEWLINE(xml, XML_CARD_VALUES_ELEMENT, cardValues);
	};

	return xml;
}

CByteArray APL_DocEId::getCSV()
{
/*
version;type;name;surname;gender;date_of_birth;location_of_birth;nobility;nationality;
	national_nr;special_organization;member_of_family;special_status;logical_nr;chip_nr;
	date_begin;date_end;issuing_municipality;version;street;zip;municipality;country;
	file_id;file_address;
*/

	CByteArray csv;

	csv+=getDocumentVersion();
	csv+=CSV_SEPARATOR;
	csv+=getDocumentType();
	csv+=CSV_SEPARATOR;
	csv+=getGivenName();
	csv+=CSV_SEPARATOR;
	csv+=getSurname();
	csv+=CSV_SEPARATOR;
	csv+=getGender();
	csv+=CSV_SEPARATOR;
	csv+=getDateOfBirth();
	csv+=CSV_SEPARATOR;
	csv+=CSV_SEPARATOR;
	csv+=getNationality();
	csv+=CSV_SEPARATOR;
	csv+=getCivilianIdNumber();
	csv+=CSV_SEPARATOR;

	csv+=getDocumentPAN();
	csv+=CSV_SEPARATOR;
	csv+=getValidityBeginDate();
	csv+=CSV_SEPARATOR;
	csv+=getValidityEndDate();
	csv+=CSV_SEPARATOR;

	csv+=getMRZ1();
	csv+=CSV_SEPARATOR;
	csv+=getMRZ2();
	csv+=CSV_SEPARATOR;
	csv+=getMRZ3();
	csv+=CSV_SEPARATOR;


	CByteArray baFileB64;
	if(m_cryptoFwk->b64Encode(m_card->getFileID()->getData(),baFileB64,false))
		csv+=baFileB64;
	csv+=CSV_SEPARATOR;

	return csv;
}

CByteArray APL_DocEId::getTLV()
{
	CTLVBuffer tlv;

	tlv.SetTagData(PTEID_TLV_TAG_FILE_ID,m_card->getFileID()->getData().GetBytes(),m_card->getFileID()->getData().Size());

	unsigned long ulLen=tlv.GetLengthNeeded();
	unsigned char *pucData= new unsigned char[ulLen];
	tlv.Extract(pucData,ulLen);
	CByteArray ba(pucData,ulLen);

	delete[] pucData;

	return ba;
}

const char *APL_DocEId::getValidation()
{
	return m_card->getFileTrace()->getValidation();
}

const char *APL_DocEId::getDocumentVersion()
{
	return m_card->getFileID()->getDocumentVersion();
}

const char *APL_DocEId::getDocumentType()
{
	return m_card->getFileID()->getDocumentType();
}

const char *APL_DocEId::getCountry()
{
	return m_card->getFileID()->getCountry();
}

const char *APL_DocEId::getGivenName()
{
	return m_card->getFileID()->getGivenName();
}

const char *APL_DocEId::getSurname()
{
	return m_card->getFileID()->getSurname();
}

const char *APL_DocEId::getGender()
{
	return m_card->getFileID()->getGender();
}

const char *APL_DocEId::getDateOfBirth()
{
	return m_card->getFileID()->getDateOfBirth();
}

const char *APL_DocEId::getNationality()
{
	return m_card->getFileID()->getNationality();
}

const char *APL_DocEId::getDocumentPAN()
{
	return m_card->getFileID()->getDocumentPAN();
}

const char *APL_DocEId::getValidityBeginDate()
{
	return m_card->getFileID()->getValidityBeginDate();
}

const char *APL_DocEId::getValidityEndDate()
{
	return m_card->getFileID()->getValidityEndDate();
}

const char *APL_DocEId::getHeight()
{
	return m_card->getFileID()->getHeight();
}

const char *APL_DocEId::getDocumentNumber()
{
	return m_card->getFileID()->getDocumentNumber();
}

const char *APL_DocEId::getTaxNo()
{
	return m_card->getFileID()->getTaxNo();
}

const char *APL_DocEId::getSocialSecurityNumber()
{
	return m_card->getFileID()->getSocialSecurityNumber();
}

const char *APL_DocEId::getHealthNumber()
{
	return m_card->getFileID()->getHealthNumber();
}

const char *APL_DocEId::getIssuingEntity()
{
	return m_card->getFileID()->getIssuingEntity();
}

const char *APL_DocEId::getLocalofRequest()
{
	return m_card->getFileID()->getLocalofRequest();
}

const char *APL_DocEId::getGivenNameFather()
{
	return m_card->getFileID()->getGivenNameFather();
}

const char *APL_DocEId::getSurnameFather()
{
	return m_card->getFileID()->getSurnameFather();
}

const char *APL_DocEId::getGivenNameMother()
{
	return m_card->getFileID()->getGivenNameMother();
}

const char *APL_DocEId::getSurnameMother()
{
	return m_card->getFileID()->getSurnameMother();
}

const char *APL_DocEId::getParents()
{
	return m_card->getFileID()->getParents();
}

PhotoPteid *APL_DocEId::getPhotoObj()
{
	return m_card->getFileID()->getPhotoObj();
}

APLPublicKey *APL_DocEId::getCardAuthKeyObj(){
	return m_card->getFileID()->getCardAuthKeyObj();
}

const char *APL_DocEId::getMRZ1(){
	return m_card->getFileID()->getMRZ1();
}

const char *APL_DocEId::getMRZ2(){
	return m_card->getFileID()->getMRZ2();
}

const char *APL_DocEId::getMRZ3(){
	return m_card->getFileID()->getMRZ3();
}

const char *APL_DocEId::getAccidentalIndications(){
	return m_card->getFileID()->getAccidentalIndications();
}

const char *APL_DocEId::getCivilianIdNumber(){
	return m_card->getFileID()->getCivilianIdNumber();
}

/*****************************************************************************************
----------------------------------- APL_PersonalNotesEid ----------------------------------------
*****************************************************************************************/
APL_PersonalNotesEId::APL_PersonalNotesEId(APL_EIDCard *card)
{
	m_card=card;
}

APL_PersonalNotesEId::~APL_PersonalNotesEId()
{
}


CByteArray APL_PersonalNotesEId::getXML(bool bNoHeader, APL_XmlUserRequestedInfo &xmlUInfo){
	CByteArray ca;

	_xmlUInfo = &xmlUInfo;
	ca = getXML(bNoHeader);
	_xmlUInfo = NULL;

	return ca;
}

CByteArray APL_PersonalNotesEId::getXML(bool bNoHeader)
{
	CByteArray xml;
	string str;

	if (_xmlUInfo->contains(XML_PERSONAL_NOTES)){
		str = getPersonalNotes();
		replace(str,XML_ESCAPE_AMP);
		replace(str,XML_ESCAPE_APOS);
		replace(str,XML_ESCAPE_GT);
		replace(str,XML_ESCAPE_LT);
		replace(str,XML_ESCAPE_QUOTE);

		BUILD_XML_ELEMENT(xml, XML_PERSONAL_NOTES_ELEMENT, str.c_str());
	}

	return xml;
}

CByteArray APL_PersonalNotesEId::getCSV()
{
	CByteArray csv;

	return csv;
}

CByteArray APL_PersonalNotesEId::getTLV()
{
	CTLVBuffer tlv;
	CByteArray ba;

	return ba;
}

const char *APL_PersonalNotesEId::getPersonalNotes(bool forceMap)
{
	return m_card->getFilePersoData()->getPersoData(forceMap);
}

/*****************************************************************************************
---------------------------------------- APL_AddrEId ---------------------------------------------
*****************************************************************************************/

APL_AddrEId::APL_AddrEId(APL_EIDCard *card)
{
	m_card=card;
	remoteAddressLoaded = false;
}

APL_AddrEId::~APL_AddrEId()
{
}


CByteArray APL_AddrEId::getXML(bool bNoHeader, APL_XmlUserRequestedInfo &xmlUInfo){
	CByteArray ca;

	_xmlUInfo = &xmlUInfo;
	ca = getXML(bNoHeader);
	_xmlUInfo = NULL;

	return ca;
}

CByteArray APL_AddrEId::getXML(bool bNoHeader)
{
	CByteArray xml;
	CByteArray address;
	bool addAddress = false;

	/*	if (isNationalAddress()){ //specification for xml does not include foreign addresses, this will stay here if specification changes */
	if (_xmlUInfo->contains(XML_DISTRICT)){
		BUILD_XML_ELEMENT(address, XML_DISTRICT_ELEMENT, getDistrict());
		addAddress = true;
	}
	if (_xmlUInfo->contains(XML_MUNICIPALITY)){
		BUILD_XML_ELEMENT(address, XML_MUNICIPALITY_ELEMENT, getMunicipality());
		addAddress = true;
	}
	if (_xmlUInfo->contains(XML_CIVIL_PARISH)){
		BUILD_XML_ELEMENT(address, XML_CIVIL_PARISH_ELEMENT, getCivilParish());
		addAddress = true;
	}
	if (_xmlUInfo->contains(XML_ABBR_STREET_TYPE)){
		BUILD_XML_ELEMENT(address, XML_ABBR_STREET_TYPE_ELEMENT, getAbbrStreetType());
		addAddress = true;
	}
	if (_xmlUInfo->contains(XML_STREET_TYPE)){
		BUILD_XML_ELEMENT(address, XML_STREET_TYPE_ELEMENT, getStreetType());
		addAddress = true;
	}
	if (_xmlUInfo->contains(XML_STREET_NAME)){
		BUILD_XML_ELEMENT(address, XML_STREET_NAME_ELEMENT, getStreetName());
		addAddress = true;
	}
	if (_xmlUInfo->contains(XML_ABBR_BUILDING_TYPE)){
		BUILD_XML_ELEMENT(address, XML_ABBR_BUILDING_TYPE_ELEMENT, getAbbrBuildingType());
		addAddress = true;
	}
	if (_xmlUInfo->contains(XML_BUILDING_TYPE)){
		BUILD_XML_ELEMENT(address, XML_BUILDING_TYPE_ELEMENT, getBuildingType());
		addAddress = true;
	}
	if (_xmlUInfo->contains(XML_DOOR_NO)){
		BUILD_XML_ELEMENT(address, XML_DOOR_NO_ELEMENT, getDoorNo());
		addAddress = true;
	}
	if (_xmlUInfo->contains(XML_FLOOR)){
		BUILD_XML_ELEMENT(address, XML_FLOOR_ELEMENT, getFloor());
		addAddress = true;
	}
	if (_xmlUInfo->contains(XML_SIDE)){
		BUILD_XML_ELEMENT(address, XML_SIDE_ELEMENT, getSide());
		addAddress = true;
	}
	if (_xmlUInfo->contains(XML_PLACE)){
		BUILD_XML_ELEMENT(address, XML_PLACE_ELEMENT, getPlace());
		addAddress = true;
	}
	if (_xmlUInfo->contains(XML_LOCALITY)){
		BUILD_XML_ELEMENT(address, XML_LOCALITY_ELEMENT, getLocality());
		addAddress = true;
	}
	if (_xmlUInfo->contains(XML_ZIP4)){
		BUILD_XML_ELEMENT(address, XML_ZIP4_ELEMENT, getZip4());
		addAddress = true;
	}
	if (_xmlUInfo->contains(XML_ZIP3)){
		BUILD_XML_ELEMENT(address, XML_ZIP3_ELEMENT, getZip3());
		addAddress = true;
	}
	if (_xmlUInfo->contains(XML_POSTAL_LOCALITY)){
		BUILD_XML_ELEMENT(address, XML_POSTAL_LOCALITY_ELEMENT, getPostalLocality());
		addAddress = true;
	}
	if (addAddress){
		BUILD_XML_ELEMENT_NEWLINE(xml,XML_ADDRESS_ELEMENT, address);
	}

	/* } else { //specification for xml does not include foreign addresses, this will stay here if specification changes
		if (_xmlUInfo->contains(XML_FOREIGN_COUNTRY)){
			BUILD_XML_ELEMENT(address, XML_FOREIGN_COUNTRY_ELEMENT, getForeignCountry());
			addAddress = true;
		}
		if (_xmlUInfo->contains(XML_FOREIGN_ADDRESS)){
			BUILD_XML_ELEMENT(address, XML_FOREIGN_ADDRESS_ELEMENT, getForeignAddress());
			addAddress = true;
		}
		if (_xmlUInfo->contains(XML_FOREIGN_CITY)){
			BUILD_XML_ELEMENT(address, XML_FOREIGN_CITY_ELEMENT, getForeignCity());
			addAddress = true;
		}
		if (_xmlUInfo->contains(XML_FOREIGN_REGION)){
			BUILD_XML_ELEMENT(address, XML_FOREIGN_REGION_ELEMENT, getForeignRegion());
			addAddress = true;
		}
		if (_xmlUInfo->contains(XML_FOREIGN_LOCALITY)){
			BUILD_XML_ELEMENT(address, XML_FOREIGN_LOCALITY_ELEMENT, getForeignLocality());
			addAddress = true;
		}
		if (_xmlUInfo->contains(XML_FOREIGN_POSTAL_CODE)){
			BUILD_XML_ELEMENT(address, XML_FOREIGN_POSTAL_CODE_ELEMENT, getForeignPostalCode());
			addAddress = true;
		}
	}
	 */
	return xml;
}

CByteArray APL_AddrEId::getCSV()
{
	CByteArray csv;

	/*
	CByteArray baFileB64;
	if(m_cryptoFwk->b64Encode(m_card->getFileAddress()->getData(),baFileB64,false))
		csv+=baFileB64;
	*/
	return csv;
}

CByteArray APL_AddrEId::getTLV()
{
	CTLVBuffer tlv;

	tlv.SetTagData(PTEID_TLV_TAG_FILE_ADDR,m_card->getFileAddress()->getData().GetBytes(),m_card->getFileAddress()->getData().Size());

	unsigned long ulLen=tlv.GetLengthNeeded();
	unsigned char *pucData= new unsigned char[ulLen];
	tlv.Extract(pucData,ulLen);
	CByteArray ba(pucData,ulLen);

	delete[] pucData;

	return ba;
}


bool checkResultSW12(CByteArray &result, unsigned int *p_sw12)
{
	unsigned long ulRespLen = result.Size();
	const unsigned char *result_ptr = result.GetBytes();

	unsigned int ulSW12 = (unsigned int)(256 * result_ptr[ulRespLen - 2] + result_ptr[ulRespLen - 1]);
	*p_sw12 = ulSW12;

	return ulSW12 == 0x9000;
}

void json_parse_string(std::string &dest_string, cJSON *json_obj, const char *item_name) {

	cJSON * item = cJSON_GetObjectItem(json_obj, item_name);

	if (cJSON_IsString(item)) {
		dest_string.append(item->valuestring);
	}
	else {
		MWLOG(LEV_ERROR, MOD_APL, "Failed to parse JSON string element: %s", item_name);
	}

}

void APL_AddrEId::mapNationalFields(cJSON * json_obj) {

	
	m_AddressType.append("N");

	json_parse_string(m_CountryCode, json_obj, "countryCode");
	json_parse_string(m_DistrictCode, json_obj, "districtCode");
	json_parse_string(m_DistrictName, json_obj, "district");
	json_parse_string(m_MunicipalityCode, json_obj, "municipalityCode");
	json_parse_string(m_MunicipalityName, json_obj, "municipality");
	json_parse_string(m_CivilParishCode, json_obj, "parishCode");
	json_parse_string(m_CivilParishName, json_obj, "parish");
	json_parse_string(m_AbbrStreetType,  json_obj, "abbrStreetType");
	json_parse_string(m_StreetType,      json_obj, "streetType");
	json_parse_string(m_StreetName,      json_obj, "streetName");
	json_parse_string(m_AbbrBuildingType, json_obj, "abbrBuildingType");
	json_parse_string(m_BuildingType, json_obj, "buildingType");
	json_parse_string(m_DoorNo, json_obj, "doorNo");
	json_parse_string(m_Floor, json_obj, "floor");
	json_parse_string(m_Side, json_obj, "side");
	json_parse_string(m_Locality, json_obj, "locality");
	json_parse_string(m_Zip4, json_obj, "zip4");
	json_parse_string(m_Zip3, json_obj, "zip3");
	json_parse_string(m_PostalLocality, json_obj, "postalLocality");
	json_parse_string(m_Place, json_obj, "place");
	json_parse_string(m_Generated_Address_Code, json_obj, "generatedAddressCode");
}


void APL_AddrEId::mapForeignFields(cJSON * json_obj) {

	m_AddressType.append("I");
	json_parse_string(m_CountryCode, json_obj, "countryCode");

	json_parse_string(m_Foreign_Country, json_obj,     "foreignCountry");
	json_parse_string(m_Foreign_City, json_obj,        "foreignCity");
	json_parse_string(m_Foreign_Region, json_obj,      "foreignRegion");
	json_parse_string(m_Foreign_Locality, json_obj,    "foreignLocality");
	json_parse_string(m_Foreign_Postal_Code, json_obj, "foreignPostalCode");
	json_parse_string(m_Generated_Address_Code, json_obj, "generatedAddressCode");
	json_parse_string(m_Foreign_Generic_Address, json_obj, "foreignGenericAddress");

}

void APL_AddrEId::loadRemoteAddress() {
	const std::string ENDPOINT_DH = "/readaddress/sendDHParams";
    const std::string ENDPOINT_SIGNCHALLENGE = "/readaddress/signChallenge";
    const std::string ENDPOINT_READADDRESS = "/readaddress/readAddress";

    std::string url_endpoint_dh, url_endpoint_signchallenge, url_endpoint_readaddress;
    long exception_code = 0;

	if (remoteAddressLoaded) {
		return;
	}

	APL_Config conf_baseurl(CConfig::EIDMW_CONFIG_PARAM_GENERAL_REMOTEADDR_BASEURL);

	if (!CConfig::isTestModeEnabled()) {
		url_endpoint_dh += conf_baseurl.getString();
		url_endpoint_readaddress += conf_baseurl.getString();
		url_endpoint_signchallenge += conf_baseurl.getString();
	}
	else {
		APL_Config conf_baseurl_t(CConfig::EIDMW_CONFIG_PARAM_GENERAL_REMOTEADDR_BASEURL_TEST);
		url_endpoint_dh += conf_baseurl_t.getString();
		url_endpoint_readaddress += conf_baseurl_t.getString();
		url_endpoint_signchallenge += conf_baseurl_t.getString();
	}

	url_endpoint_dh += ENDPOINT_DH;
	url_endpoint_signchallenge += ENDPOINT_SIGNCHALLENGE;
	url_endpoint_readaddress += ENDPOINT_READADDRESS;
	

	SAM sam_helper(m_card);
	DHParams dh_params;

	CByteArray sod_data = getSodData(m_card);
	CByteArray authCert_data = getAuthCert(m_card);

	sam_helper.getDHParams(&dh_params, true);


	char * json_str = build_json_obj_dhparams(dh_params, m_card->getFileID(), m_card->getFileAddress(), sod_data, authCert_data);

	//cerr << "json_str: " << endl << json_str << endl;

	//1st POST
	PostResponse resp = post_json_remoteaddress(url_endpoint_dh.c_str(), json_str, NULL);

	MWLOG(LEV_INFO, MOD_APL, "%s Endpoint (1) returned HTTP code: %ld", __FUNCTION__, resp.http_code);

	std::string received_cookie = parseCookieFromHeaders(resp.http_headers);

	//cout << "Parsed cookie: " << received_cookie << endl;

	RA_DHParamsResponse dh_params_resp = parseDHParamsResponse(resp.http_response.c_str());

	if (dh_params_resp.error_code == 0)
	{
		if (!sam_helper.sendKIFD((char *)dh_params_resp.kifd.c_str())) {
			MWLOG(LEV_ERROR, MOD_APL, "loadRemoteAddress(): Card failed to accept DH server public key KIFD!");
			exception_code = EIDMW_REMOTEADDR_SMARTCARD_ERROR;
			goto cleanup;
		}

    	//2nd POST
		char * kicc = sam_helper.getKICC();

		bool verified = sam_helper.verifyCert_CV_IFD((char *)dh_params_resp.cv_ifd_cert.c_str());

		if (!verified) {
			MWLOG(LEV_ERROR, MOD_APL, "Card failed to verify server-provided CV certificate!");
			exception_code = EIDMW_REMOTEADDR_SMARTCARD_ERROR;
			goto cleanup;
		}

		char * chr = sam_helper.getPK_IFD_AUT((char *)dh_params_resp.cv_ifd_cert.c_str());

		//prepareExternalAutenticate(card, cvc_cert, rsa_ca_pubkey);

		char * challenge = sam_helper.generateChallenge(chr);
		if (challenge == NULL) {
			exception_code = EIDMW_REMOTEADDR_SMARTCARD_ERROR;
			goto cleanup;
		}

		json_str = build_json_obj_sign_challenge(challenge, kicc);
		PostResponse resp = post_json_remoteaddress(url_endpoint_signchallenge.c_str(), json_str, received_cookie.c_str());

		MWLOG(LEV_INFO, MOD_APL, "%s Endpoint (2) returned HTTP code: %ld", __FUNCTION__, resp.http_code);


		RA_SignChallengeResponse signed_challenge_obj = 
		parseSignChallengeResponse(resp.http_response.c_str());

		if (signed_challenge_obj.error_code == 0) {
			char * signed_challenge = (char *) signed_challenge_obj.signed_challenge.c_str();
			bool ret = sam_helper.verifySignedChallenge(signed_challenge);

			if (!ret) {
				MWLOG(LEV_ERROR, MOD_APL, "Card rejected server-provided signature. Process aborted!");
				exception_code = EIDMW_REMOTEADDR_SMARTCARD_ERROR;
				goto cleanup;
			}

			MWLOG(LEV_DEBUG, MOD_APL, "From now on using Secure Messaging with the smartcard...");
			bool is_hex = true;
			CByteArray mse_internal_auth_cmd(signed_challenge_obj.set_se_command, is_hex); 
			CByteArray internal_auth_cmd(signed_challenge_obj.internal_auth_command, is_hex);

			const unsigned char le_byte[] = { 0x00 };

            //Needed for T=1 smartcard protocol
			mse_internal_auth_cmd.Append(le_byte, sizeof(le_byte));
			internal_auth_cmd.Append(le_byte, sizeof(le_byte));

			//TODO: maybe catch cardlayer exceptions in sendAPDU and translate to SMARTCARD_ERROR

			CByteArray resp_mse_internal_auth = m_card->sendAPDU(mse_internal_auth_cmd);

			unsigned int sw12 = 0;
			if (!checkResultSW12(resp_mse_internal_auth, &sw12)) {
				MWLOG(LEV_ERROR, MOD_APL, "MSE SET INTERNAL Auth failed! SW12: %04X", sw12);
				exception_code = EIDMW_REMOTEADDR_SMARTCARD_ERROR;
				goto cleanup;

			}

			CByteArray resp_internal_auth = m_card->sendAPDU(internal_auth_cmd);
			sw12 = 0;
			if (!checkResultSW12(resp_internal_auth, &sw12)) {
				MWLOG(LEV_ERROR, MOD_APL, "INTERNAL AUTHENTICATION failed! SW12: %04X", sw12);
				exception_code = EIDMW_REMOTEADDR_SMARTCARD_ERROR;
				goto cleanup;

			}

			json_str = build_json_obj_read_address(resp_mse_internal_auth, resp_internal_auth);
			resp = post_json_remoteaddress(url_endpoint_readaddress.c_str(), json_str, received_cookie.c_str());
            
			MWLOG(LEV_INFO, MOD_APL, "%s Endpoint (3) returned HTTP code: %ld", __FUNCTION__, resp.http_code);

			RA_GetAddressResponse *getaddr_resp = validateReadAddressResponse(resp.http_response.c_str());
			if (getaddr_resp->address_obj != NULL) {
				remoteAddressLoaded = true;
				if (!getaddr_resp->is_foreign_address) {
					mapNationalFields(getaddr_resp->address_obj);
				}
				else {
					mapForeignFields(getaddr_resp->address_obj);
				}

			}
			else {

				MWLOG(LEV_ERROR, MOD_APL, "Unexpected server response for %s, no http error code but empty/malformed response", ENDPOINT_READADDRESS.c_str());
				exception_code = EIDMW_REMOTEADDR_SERVER_ERROR;
			}

		}

	}

	cleanup:
	  if (exception_code > 0) {
	  	throw CMWEXCEPTION(exception_code);
	  }

}

const char *APL_AddrEId::getMunicipality()
{

	loadRemoteAddress();
	return m_MunicipalityName.c_str();
}

const char *APL_AddrEId::getMunicipalityCode()
{	
	loadRemoteAddress();
	return m_MunicipalityCode.c_str();
}

const char *APL_AddrEId::getPlace()
{
	loadRemoteAddress();
	return m_Place.c_str();
}

const char *APL_AddrEId::getCivilParish()
{
	loadRemoteAddress();
	return m_CivilParishName.c_str();
}

const char *APL_AddrEId::getCivilParishCode()
{
	loadRemoteAddress();
	return m_CivilParishCode.c_str();
}

const char *APL_AddrEId::getStreetName()
{
	loadRemoteAddress();
	return m_StreetName.c_str();
}

const char *APL_AddrEId::getAbbrStreetType()
{
	loadRemoteAddress();
	return m_AbbrStreetType.c_str();
}

const char *APL_AddrEId::getStreetType()
{
	loadRemoteAddress();
	return m_StreetType.c_str();
}

const char *APL_AddrEId::getAbbrBuildingType()
{
	loadRemoteAddress();
	return m_AbbrBuildingType.c_str();
}

const char *APL_AddrEId::getBuildingType()
{
	loadRemoteAddress();
	return m_BuildingType.c_str();
}

const char *APL_AddrEId::getDoorNo()
{
	loadRemoteAddress();
	return m_DoorNo.c_str();
}

const char *APL_AddrEId::getFloor()
{
	loadRemoteAddress();
	return m_Floor.c_str();
}

const char *APL_AddrEId::getSide()
{
	loadRemoteAddress();
	return m_Side.c_str();
}

const char *APL_AddrEId::getLocality()
{
	loadRemoteAddress();
	return m_Locality.c_str();
}

const char *APL_AddrEId::getZip4()
{
	loadRemoteAddress();
	return m_Zip4.c_str();
}

const char *APL_AddrEId::getZip3()
{
	loadRemoteAddress();
	return m_Zip3.c_str();
}

const char *APL_AddrEId::getPostalLocality()
{
	loadRemoteAddress();
	return m_PostalLocality.c_str();
}

const char *APL_AddrEId::getGeneratedAddressCode()
{
	loadRemoteAddress();
	return m_Generated_Address_Code.c_str();
}

const char *APL_AddrEId::getDistrict()
{
	loadRemoteAddress();
	return m_DistrictName.c_str();
}

const char *APL_AddrEId::getDistrictCode()
{
	loadRemoteAddress();
	return m_DistrictCode.c_str();
}

const char *APL_AddrEId::getCountryCode()
{
	loadRemoteAddress();
	return m_CountryCode.c_str();
}

bool APL_AddrEId::isNationalAddress()
{
	loadRemoteAddress();
	return m_AddressType == "N";
}

const char *APL_AddrEId::getForeignCountry()
{
	loadRemoteAddress();
	return m_Foreign_Country.c_str();
}

const char *APL_AddrEId::getForeignAddress()
{
	loadRemoteAddress();
	return m_Foreign_Generic_Address.c_str();
}

const char *APL_AddrEId::getForeignCity()
{
	loadRemoteAddress();
	return m_Foreign_City.c_str();
}

const char *APL_AddrEId::getForeignRegion()
{
	loadRemoteAddress();
	return m_Foreign_Region.c_str();
}

const char *APL_AddrEId::getForeignLocality()
{
	loadRemoteAddress();
	return m_Foreign_Locality.c_str();
}

const char *APL_AddrEId::getForeignPostalCode()
{
	loadRemoteAddress();
	return m_Foreign_Postal_Code.c_str();
}

/*****************************************************************************************
---------------------------------------- APL_SodEid -----------------------------------------
*****************************************************************************************/
APL_SodEid::APL_SodEid(APL_EIDCard *card)
{
	m_card=card;
}

APL_SodEid::~APL_SodEid()
{
}

CByteArray APL_SodEid::getXML(bool bNoHeader)
{

	CByteArray xml;
	CByteArray baB64;

	if(!bNoHeader)
		xml+="<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";

	xml+="<biometric>\n";
	xml+="	<picture type=\"jpg\">\n";
	xml+="		<data encoding=\"base64\">\n";
	if(m_cryptoFwk->b64Encode(getData(),baB64))
		xml+=		baB64;
	xml+="		</data>\n";
	xml+="		<hash encoding=\"base64\" method=\"md5\">\n";
	xml+="		</hash>\n";
	xml+="	</picture>\n";
	xml+="</biometric>\n";

	return xml;
}

CByteArray APL_SodEid::getCSV()
{
	CByteArray csv;
	CByteArray baB64;

	if(m_cryptoFwk->b64Encode(getData(),baB64,false))
		csv+=		baB64;
	csv+=CSV_SEPARATOR;
	csv+=CSV_SEPARATOR;

	return csv;
}

CByteArray APL_SodEid::getTLV()
{
	CTLVBuffer tlv;

	tlv.SetTagData(PTEID_TLV_TAG_FILE_SOD,m_card->getFileSod()->getData().GetBytes(),m_card->getFileSod()->getData().Size());

	unsigned long ulLen=tlv.GetLengthNeeded();
	unsigned char *pucData= new unsigned char[ulLen];
	tlv.Extract(pucData,ulLen);
	CByteArray ba(pucData,ulLen);

	delete[] pucData;

	return ba;
}

const CByteArray& APL_SodEid::getData()
{
	const CByteArray &cb = m_card->getFileSod()->getData();

	m_card->getFileSod()->getAddressHash();

	return cb;
}

/*****************************************************************************************
---------------------------------------- APL_DocVersionInfo --------------------------------------------
*****************************************************************************************/
APL_DocVersionInfo::APL_DocVersionInfo(APL_EIDCard *card)
{
	m_card=card;
}

APL_DocVersionInfo::~APL_DocVersionInfo()
{
}

CByteArray APL_DocVersionInfo::getXML(bool bNoHeader)
{
/*
	<scard>
		<serial_nr></serial_nr>
		<component_code></component_code>
		<os_nr></os_nr>
		<os_version></os_version>
		<softmask_nr></softmask_nr>
		<softmask_version></softmask_version>
		<applet_version></applet_version>
		<global_os_version></global_os_version>
		<applet_interface_version></applet_interface_version>
		<PKCS1_support></PKCS1_support>
		<key_exchange_version></key_exchange_version>
		<application_lifecycle></application_lifecycle>
		<graph_perso></graph_perso>
		<elec_perso></elec_perso>
		<elec_perso_interface></elec_perso_interface>
		<files>
			<file_datainfo encoding="base64">
			</file_datainfo>
			<file_tokeninfo encoding="base64">
			</file_tokeninfo>
		</files>
	</scard>
*/

	CByteArray xml;
	CByteArray b64;

	if(!bNoHeader)
		xml+="<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";

	xml+="<scard>\n";
	xml+="	<serial_nr>";
	xml+=		getSerialNumber();
	xml+=	"</serial_nr>\n";
	xml+="	<component_code>";
	xml+=		getComponentCode();
	xml+=	"</component_code>\n";
	xml+="	<os_nr>";
	xml+=		getOsNumber();
	xml+=	"</os_nr>\n";
	xml+="	<os_version>";
	xml+=		getOsVersion();
	xml+=	"</os_version>\n";
	xml+="	<softmask_nr>";
	xml+=		getSoftmaskNumber();
	xml+=	"</softmask_nr>\n";
	xml+="	<softmask_version>";
	xml+=		getSoftmaskVersion();
	xml+=	"</softmask_version>\n";
	xml+="	<applet_version>";
	xml+=		getAppletVersion();
	xml+=	"</applet_version>\n";
	xml+="	<global_os_version>";
	xml+=		getGlobalOsVersion();
	xml+=	"</global_os_version>\n";
	xml+="	<applet_interface_version>";
	xml+=		getAppletInterfaceVersion();
	xml+=	"</applet_interface_version>\n";
	xml+="	<PKCS1_support>";
	xml+=		getPKCS1Support();
	xml+=	"</PKCS1_support>\n";
	xml+="	<key_exchange_version>";
	xml+=		getKeyExchangeVersion();
	xml+=	"</key_exchange_version>\n";
	xml+="	<application_lifecycle>";
	xml+=		getAppletLifeCicle();
	xml+=	"</application_lifecycle>\n";
	xml+="	<graph_perso>";
	xml+=		getGraphicalPersonalisation();
	xml+=	"</graph_perso>\n";
	xml+="	<elec_perso>";
	xml+=		getElectricalPersonalisation();
	xml+=	"</elec_perso>\n";
	xml+="	<elec_perso_interface>";
	xml+=		getElectricalPersonalisationInterface();
	xml+=	"</elec_perso_interface>\n";
	xml+="	<files>\n";
	xml+="		<file_datainfo encoding=\"base64\">\n";
	if(m_cryptoFwk->b64Encode(m_card->getFileInfo()->getData(),b64))
		xml+=b64;
	xml+="		</file_datainfo>\n";
	xml+="		<file_tokeninfo encoding=\"base64\">\n";
	if(m_cryptoFwk->b64Encode(m_card->getFileTokenInfo()->getData(),b64))
		xml+=b64;
	xml+="		</file_tokeninfo>\n";
	xml+="	</files>\n";
	xml+="</scard>\n";

	return xml;
}

CByteArray APL_DocVersionInfo::getCSV()
{
/*
serial_nr;component_code;os_nr;os_version;softmask_nr;softmask_version;applet_version;
	global_os_version;applet_interface_version;PKCS1_support;key_exchange_version;
	application_lifecycle;graph_perso;elec_perso;elec_perso_interface;
*/

	CByteArray csv;
	CByteArray b64;

	csv+=getSerialNumber();
	csv+=CSV_SEPARATOR;
	csv+=getComponentCode();
	csv+=CSV_SEPARATOR;
	csv+=getOsNumber();
	csv+=CSV_SEPARATOR;
	csv+=getOsVersion();
	csv+=CSV_SEPARATOR;
	csv+=getSoftmaskNumber();
	csv+=CSV_SEPARATOR;
	csv+=getSoftmaskVersion();
	csv+=CSV_SEPARATOR;
	csv+=getAppletVersion();
	csv+=CSV_SEPARATOR;
	csv+=getGlobalOsVersion();
	csv+=CSV_SEPARATOR;
	csv+=getAppletInterfaceVersion();
	csv+=CSV_SEPARATOR;
	csv+=getPKCS1Support();
	csv+=CSV_SEPARATOR;
	csv+=getKeyExchangeVersion();
	csv+=CSV_SEPARATOR;
	csv+=getAppletLifeCicle();
	csv+=CSV_SEPARATOR;
	csv+=getGraphicalPersonalisation();
	csv+=CSV_SEPARATOR;
	csv+=getElectricalPersonalisation();
	csv+=CSV_SEPARATOR;
	csv+=getElectricalPersonalisationInterface();
	csv+=CSV_SEPARATOR;

	if(m_cryptoFwk->b64Encode(m_card->getFileInfo()->getData(),b64,false))
		csv+=b64;
	csv+=CSV_SEPARATOR;

	if(m_cryptoFwk->b64Encode(m_card->getFileTokenInfo()->getData(),b64,false))
		csv+=b64;
	csv+=CSV_SEPARATOR;

	return csv;
}

CByteArray APL_DocVersionInfo::getTLV()
{
	CTLVBuffer tlv;

	tlv.SetTagData(PTEID_TLV_TAG_FILE_CARDINFO,m_card->getFileInfo()->getData().GetBytes(),m_card->getFileInfo()->getData().Size());
	tlv.SetTagData(PTEID_TLV_TAG_FILE_TOKENINFO,m_card->getFileTokenInfo()->getData().GetBytes(),m_card->getFileTokenInfo()->getData().Size());

	unsigned long ulLen=tlv.GetLengthNeeded();
	unsigned char *pucData= new unsigned char[ulLen];
	tlv.Extract(pucData,ulLen);
	CByteArray ba(pucData,ulLen);

	delete[] pucData;

	return ba;
}

const char *APL_DocVersionInfo::getSerialNumber()
{
	return m_card->getTokenSerialNumber();
}

const char * APL_DocVersionInfo::getTokenLabel(){
	return m_card->getTokenLabel();
}

const char *APL_DocVersionInfo::getComponentCode()
{
	return m_card->getFileInfo()->getComponentCode();
}

const char *APL_DocVersionInfo::getOsNumber()
{
	return m_card->getFileInfo()->getOsNumber();
}

const char *APL_DocVersionInfo::getOsVersion()
{
	return m_card->getFileInfo()->getOsVersion();
}

const char *APL_DocVersionInfo::getSoftmaskNumber()
{
	return m_card->getFileInfo()->getSoftmaskNumber();
}

const char *APL_DocVersionInfo::getSoftmaskVersion()
{
	return m_card->getFileInfo()->getSoftmaskVersion();
}

const char *APL_DocVersionInfo::getAppletVersion()
{
	return m_card->getAppletVersion();
}

const char *APL_DocVersionInfo::getGlobalOsVersion()
{
	return m_card->getFileInfo()->getGlobalOsVersion();
}

const char *APL_DocVersionInfo::getAppletInterfaceVersion()
{
	return m_card->getFileInfo()->getAppletInterfaceVersion();
}

const char *APL_DocVersionInfo::getPKCS1Support()
{
	return m_card->getFileInfo()->getPKCS1Support();
}

const char *APL_DocVersionInfo::getKeyExchangeVersion()
{
	return m_card->getFileInfo()->getKeyExchangeVersion();
}

const char *APL_DocVersionInfo::getAppletLifeCicle()
{
	return m_card->getFileInfo()->getAppletLifeCicle();
}

const char *APL_DocVersionInfo::getGraphicalPersonalisation()
{
	return m_card->getFileTokenInfo()->getGraphicalPersonalisation();
}

const char *APL_DocVersionInfo::getElectricalPersonalisation()
{
	return m_card->getFileTokenInfo()->getElectricalPersonalisation();
}

const char *APL_DocVersionInfo::getElectricalPersonalisationInterface()
{
	return m_card->getFileTokenInfo()->getElectricalPersonalisationInterface();
}
}
