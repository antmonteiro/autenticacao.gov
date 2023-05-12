// Constants.js
.pragma library

// Program options
var USE_SDK_PIN_UI_POPUP = true

// Menu static entries
// Changing the MainMenuModel may force update the next constants:
var MAIN_MENU_SIGN_PAGE_INDEX = 1
var SUB_MENU_SIGN_PAGE_INDEX = 0
var SIGNATURE_PAGE_URL = "contentPages/services/PageServicesSign.qml"

// Start Popup options
var DIALOG_WIDTH = 600
var DIALOG_HEIGHT = 300
var DIALOG_CASCATE_X = 10
var DIALOG_CASCATE_Y = 10
var DIALOG_CASCATE_TOP_TOP = 3
var DIALOG_CASCATE_TOP = 2
var DIALOG_CASCATE_MIDDLE = 1
var DIALOG_CASCATE_BOTTON = 0

// Border
var APP_BORDER = 1

// Error Constants
var TRIES_LEFT_ERROR = 1000
var UNSUPPORTED_PDF_ERROR = -1
var ENCRYPTED_PDF_ERROR = -2
var XFA_FORM_PDF_ERROR = -3

//PDF seal dimensions
const SIG_WIDTH_DEFAULT = 178
const SIG_WIDTH_MINIMUM = 120
const SIG_HEIGHT_DEFAULT = 90
const SIG_HEIGHT_REDUCED = 45
const SIG_HEIGHT_MINIMUM = 35



// Certificates Status
var PTEID_CERTIF_STATUS_UNKNOWN = 0     /* Validity unknown */
var PTEID_CERTIF_STATUS_REVOKED = 1     /* Revoked certificate */
var PTEID_CERTIF_STATUS_SUSPENDED = 2
var PTEID_CERTIF_STATUS_CONNECT = 3     /* Connection problem */
var PTEID_CERTIF_STATUS_ISSUER = 4      /* An issuer is missing in the chain */
var PTEID_CERTIF_STATUS_ERROR = 5       /* Error during validation */
var PTEID_CERTIF_STATUS_VALID = 6       /* Valid certificate */
var PTEID_CERTIF_STATUS_EXPIRED = 7       /* Expired certificate */

// Screen Size
var SCREEN_MINIMUM_WIDTH = 1024
var SCREEN_MINIMUM_HEIGHT = 576;

// Title bar
var TITLE_BAR_SIZE = 25
var TITLE_BAR_H_ICON_SPACE = 20

// Frame detection
var FRAME_WINDOW_SIZE = 4

// Opacity Main vs Popups Focus
var OPACITY_POPUP_FOCUS = 0.5
var OPACITY_MAIN_FOCUS = 1.0
var FOCUS_BORDER = 5

// Views Relative Size
var MAIN_MENU_VIEW_RELATIVE_SIZE = 0.25
var SUB_MENU_VIEW_RELATIVE_SIZE = 0.25
var SUB_MENU_EXPAND_VIEW_RELATIVE_SIZE = 0
var CONTENT_PAGES_VIEW_RELATIVE_SIZE = 0.50

// Main Menu Size
var BOTTOM_MENU_WIDTH_SIZE = 100
var IMAGE_LOGO_RELATIVE_V_POS = 0.05
var MAIN_MENU_RELATIVE_V_SIZE = 0.40
var MAIN_MENU_BOTTOM_RELATIVE_V_SIZE = 0.05
var MAIN_MENU_BOTTOM_RELATIVE_V_POS = 0.85
var MAIN_MENU_LINE_H_SIZE = 70
var MAIN_MENU_LINE_V_SIZE = 1
var IMAGE_ARROW_MAIN_MENU_RELATIVE = 0.9

// Sub Menu Size
var SUB_MENU_RELATIVE_V_ITEM_SIZE = 0.15
var SUB_MENU_RELATIVE_LINE_H_SIZE = 0.005
var IMAGE_ARROW_SUB_MENU_MARGIN = 5

// Text Size using pixelSize
var SIZE_TEXT_MAIN_MENU = 18
var SIZE_TEXT_SUB_MENU = 18
var SIZE_TEXT_TITLE = 20
var SIZE_TEXT_LABEL = 12
var SIZE_TEXT_LABEL_FOCUS = 14
var SIZE_TEXT_FIELD = 12
var SIZE_TEXT_BODY = 16
var SIZE_TEXT_LINK_LABEL = 12
var SIZE_TEXT_LINK_BODY = 16
var SIZE_TEXT_LIST_BULLET = 8
var SIZE_ARROW_INDICATOR = 20
var SIZE_ARROW_OFFSET = 1

// Buttons Size
var WIDTH_BUTTON = 150
var HEIGHT_BOTTOM_COMPONENT = 35
var HEIGHT_SIGN_BOTTOM_COMPONENT = 54

// Switch Size
var HEIGHT_SWITCH_COMPONENT = 30

// RadioButton Size
var HEIGHT_RADIO_BOTTOM_COMPONENT = 30

// Defined Colors
var COLOR_MAIN_BLUE = "#3C5DBC"
var COLOR_MAIN_BLACK = "#333333"
var COLOR_MAIN_SOFT_GRAY = "#EDEDED"
var COLOR_MAIN_MIDDLE_GRAY = "#D8E2F3"
var COLOR_MAIN_DARK_GRAY = "#9FAFDF"
var COLOR_GREY_BUTTON_BACKGROUND = "#D6D7D7"
var COLOR_TITLEBAR_DEBUG = "red" // TODO: change to more appropriate color
var COLOR_GRAY = "#808080"

// Menus Text Colors
var COLOR_TEXT_MAIN_MENU_SELECTED = COLOR_MAIN_SOFT_GRAY
var COLOR_TEXT_MAIN_MENU_DEFAULT = COLOR_MAIN_DARK_GRAY
var COLOR_TEXT_SUB_MENU_SELECTED = COLOR_MAIN_SOFT_GRAY
var COLOR_TEXT_SUB_MENU_DEFAULT = COLOR_MAIN_BLACK
var COLOR_TEXT_SUB_MENU_MOUSE_OVER = COLOR_MAIN_BLUE
var COLOR_TEXT_TITLE = COLOR_MAIN_BLUE

// Colors
var COLOR_BACKGROUND_MAIN_MENU = COLOR_MAIN_BLUE
var COLOR_BACKGROUND_SUB_MENU = COLOR_MAIN_BLUE
var COLOR_LINE_SUB_MENU = COLOR_MAIN_SOFT_GRAY
var COLOR_TEXT_BODY = COLOR_MAIN_BLACK
var COLOR_TEXT_LABEL = COLOR_MAIN_BLUE
var COLOR_ITEM_BULLETED = COLOR_MAIN_BLUE

// Forms
// Shadow
var FORM_SHADOW_OPACITY_FORM_EFFECT = 0.15
var COLOR_FORM_SHADOW = "#000000"
var FORM_SHADOW_H_OFFSET = 1.4
var FORM_SHADOW_V_OFFSET = 1.4
var FORM_SHADOW_RADIUS = 6.0
var FORM_SHADOW_SAMPLES = 20
var FORM_SHADOW_SPREAD = 0.0
// Glow
var FORM_GLOW_OPACITY_FORM_EFFECT = 0.15
var COLOR_FORM_GLOW = "#000000"
var FORM_GLOW_RADIUS = 2
var FORM_GLOW_CORNER_RADIUS = 0
var FORM_GLOW_SPREAD = 0

// Text space
var SIZE_TEXT_FIELD_H_SPACE = 10
var SIZE_TEXT_FIELD_V_SPACE = 4
var SIZE_SIGN_SEAL_TEXT_V_SPACE = 2
var SIZE_TEXT_V_SPACE = 5

var HEIGHT_TEXT_BOX = SIZE_TEXT_LABEL + SIZE_TEXT_V_SPACE + 2 * SIZE_TEXT_FIELD

// Row space
var SIZE_ROW_H_SPACE = 20
var SIZE_ROW_V_SPACE = 12
var SIZE_ROW_V_SPACE_DEFINITIONS_APP = 8

// Animation time
var ANIMATION_MOVE_VIEW = 400
var ANIMATION_CHANGE_OPACITY = 400
var ANIMATION_LISTVIEW_MOVE = 400
var ANIMATION_LISTVIEW_RESIZE = 400

// Image Constants
var SIZE_IMAGE_LOGO = 100
var SIZE_IMAGE_LOGO_CC_WIDTH = 160
var SIZE_IMAGE_LOGO_CC_HEIGHT = 35
var SIZE_IMAGE_ICON_TITLE_BAR= 15
var SIZE_IMAGE_ARROW_ACCORDION = 12
var SIZE_IMAGE_ARROW_SUB_MENU = 16
var SIZE_IMAGE_ARROW_MAIN_MENU_W = 20
var SIZE_IMAGE_ARROW_MAIN_MENU_H = 10
var SIZE_IMAGE_ARROW_MAIN_MENU = 16
var SIZE_IMAGE_BOTTOM_MENU = 25
var SIZE_IMAGE_FILE_REMOVE = 22
var SIZE_IMAGE_FILE_EXTRACT = 20
var SIZE_IMAGE_TOOLTIP = 17
var SIZE_IMAGE_SEAL_MOVE = 26
var SIZE_IMAGE_SEAL_RESIZE = 20
var SIZE_SPACE_IMAGE_TOOLTIP = 2

// Pages Constants
// Home Page
var HEIGHT_HOME_PAGE_ROW_TOP_V_RELATIVE = 0.10
var HEIGHT_HOME_PAGE_ROW_TOP_INC_RELATIVE = 0.40
// Card Identify
var WIDTH_PHOTO_IMAGE = SIZE_IMAGE_LOGO_CC_WIDTH - SIZE_ROW_H_SPACE - 28
var HEIGHT_PHOTO_IMAGE = 150
var HEIGHT_CARD_IDENTIFY_ROW_TOP_V_RELATIVE = 0.07
var HEIGHT_CARD_IDENTIFY_ROW_TOP_INC_RELATIVE = 0.20
// Card Notes
var PAGE_NOTES_MAX_NOTES_LENGHT = 1000
var PAGE_NOTES_TEXT_V_RELATIVE = 0.60
var HEIGHT_CARD_NOTES_ROW_TOP_V_RELATIVE = 0.15
var HEIGHT_CARD_NOTES_ROW_TOP_INC_RELATIVE = 0.06
var MAIN_MENU_PRESSED = 0
var SUB_MENU_PRESSED = 1
var MAIN_BOTTOM_MENU_PRESSED = 2
var HOME_ICON_PRESSED = 3
var KEY_NAVIGATION_EXIT_NOTES = 4
var QUIT_APPLICATION = -1
// Card Oher data
var HEIGHT_CARD_OTHER_DATA_ROW_TOP_V_RELATIVE = 0.15
var HEIGHT_CARD_OTHER_DATA_ROW_TOP_INC_RELATIVE = 0.07
// Card Adress
var HEIGHT_CARD_ADRESS_ROW_TOP_V_RELATIVE = 0.05
var HEIGHT_CARD_ADRESS_ROW_TOP_INC_RELATIVE = 0.20
// Services Sign
var SIZE_LISTVIEW_IMAGE_SPACE = 10
var SIZE_ASIC_LISTVIEW_TEXT_OFFSET = 40
var SIZE_LISTVIEW_SPACING = 5
var OPACITY_SERVICES_SIGN_ADVANCE_TEXT_DISABLED = 0.5
var TOOLTIP_TIMEOUT_MS = 2000
var MIN_HEIGHT_FILES_RECT = 75
var HEIGHT_HELP_EXPANDED = 160
var HEIGHT_HELP_COLLAPSED = 25
var SCAP_ATTR_LISTVIEW_CACHEBUFFER = 10000
// Security Certificates
var HEIGHT_SECURITY_CERTIFICARES_ROW_TOP_V_RELATIVE = 0.0
var HEIGHT_SECURITY_CERTIFICARES_ROW_TOP_INC_RELATIVE = 0.0
// Definitions Security
var HEIGHT_SECURITY_PIN_ROW_TOP_V_RELATIVE = 0.25
var HEIGHT_SECURITY_PIN_ROW_TOP_INC_RELATIVE = 0.11
// Definitions Signture
var OPACITY_SIGNATURE_TEXT_DISABLED = 0.7
var OPACITY_SIGNATURE_IMAGE_DISABLED = 0.3
var SIZE_MARGIN_SIGNATURE_SEAL_CONFIG = 2
// Definitions Updates
var WIDTH_DEFINITIONS_UPDATE_MAIN_H_RELATIVE = 0.95;
var HEIGHT_DEFINITIONS_UPDATES_ROW_TOP_V_RELATIVE = 0.030
var HEIGHT_DEFINITIONS_UPDATES_ROW_TOP_INC_RELATIVE = 0.15
var HEIGHT_DEFINITIONS_UPDATE_LABEL_ROW = 0.05
var HEIGHT_DEFINITIONS_UPDATE_RELEASE_NOTE = 0.8;
var HEIGHT_DEFININTIONS_UPDATE_PROGRESS_BAR = 50
var HEIGHT_DEFININTIONS_UPDATE_APP_RELATIVE = 0.65
var HEIGHT_DEFININTIONS_UPDATE_CERTS_RELATIVE = 0.35
// Definitions Aplication
var HEIGHT_DEFINITIONS_APP_ROW_TOP_V_RELATIVE = 0.030
var HEIGHT_DEFINITIONS_APP_ROW_TOP_INC_RELATIVE = 0.15
var OPACITY_DEFINITIONS_APP_OPTION_DISABLED = 0.4
// Definitions Attributes
var HEIGHT_DEFINITIONS_ATTRIBUTES_ROW_TOP_V_RELATIVE = 0.05
var HEIGHT_DEFINITIONS_ATTRIBUTES_ROW_TOP_INC_RELATIVE = 0.10
// Help Doc Online
var HEIGHT_DOC_ONLINE_ROW_TOP_V_RELATIVE = 0.10
var HEIGHT_DOC_ONLINE_ROW_TOP_INC_RELATIVE = 0.40
// Help Doc About
var HEIGHT_HELP_ABOUT_ROW_TOP_V_RELATIVE = 0.10
var HEIGHT_HELP_ABOUT_ROW_TOP_INC_RELATIVE = 0.40
// Help Accessibility
var HEIGHT_HELP_ACCESSIBILITY_ROW_TOP_V_RELATIVE = 0.07
var HEIGHT_HELP_ACCESSIBILITY_ROW_TOP_INC_RELATIVE = 0.40
// Notification Center
var MARGIN_NOTIFICATION_CENTER = 14

// Creates a JS Enum, making it impossible to update
var MenuState = Object.freeze({
    FIRST_RUN:  "STATE_FIRST_RUN",
    HOME:       "STATE_HOME",
    SUB_MENU:   "STATE_SUBMENU",
    EXPAND:     "STATE_EXPAND",
    NORMAL:     "STATE_NORMAL"
})

var DLG_STATE = Object.freeze({
    REGISTER_FORM:          "0",
    PROGRESS:               "1",
    VALIDATE_OTP:           "2",
    SHOW_MESSAGE:           "3",
    OPEN_FILE:              "4",
    OPEN_FILE_ERROR:        "5",
    LOAD_ATTRIBUTES:        "6",
    ASK_TO_REGISTER_CERT:   "7"
})

var ARROW_RIGHT = "images/arrow-right_white_AMA.png"
var ARROW_RIGHT_HOVER = "images/arrow-right_hover.png"

var FLICK_Y_VELOCITY = 250
var FLICK_Y_VELOCITY_MAX = 2500
var FLICK_Y_VELOCITY_ATTR_LIST = 400
var FLICK_Y_VELOCITY_MAX_ATTR_LIST = 860

var DIRECTION_DOWN = -1;
var DIRECTION_UP = 1;
var NO_DIRECTION = 0;

// Seal constants
var SEAL_NAME_OFFSET = 6
var SEAL_LOCATION_OFFSET = 5.5
var SEAL_PROVIDER_NAME_OFFSET = 6.5
var SEAL_ATTR_NAME_OFFSET = 9.5

var NAME_MAX_LINES = 5
var LOCATION_MAX_LINES = 2

var PROVIDER_SCAP_MAX_LINES = 2
var ATTR_SCAP_MAX_LINES = 10   
