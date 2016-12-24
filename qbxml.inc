<?php
/**
 * @file
 *
 * These includes enable outside interaction with qbxml:
 * * File access
 */

// magic strings
define( 'QBXMLMsgsRq', 'QBXMLMsgsRq' );
define( 'QBXML_FORM_SPECIAL', '_qbxml_' ); // claim a namespace for form elements for QBXML control
define( 'QBXML_FORM_REQUEST_PREFIX', QBXML_FORM_SPECIAL . 'rq_' ); // prefix a form element name with this to have it get put into the @request info
define( 'QBXML_FORM_NAMEMAP_PREFIX', 'qbxml_'); // prefix a form element name with this to map the remainder of the name directly to QBXML
define( 'QBXML_FORM_RAWQUERY', QBXML_FORM_SPECIAL . 'raw' ); // call a form element this and the query will get tacked onto the end of the QBXML request
define( 'QBXML_FORM_REQUEST_OPCODE', QBXML_FORM_SPECIAL . 'opcode' ); // set QBXML request command to this
define( 'QBXML_FORM_REQUEST_BASE', QBXML_FORM_SPECIAL . 'base' ); // root of a QBXML request command (eg. "Account, Terms, Customer, etc.)
define( 'QBXML_THEME', '#qbxml_theme' );
define( 'QBXML_MAP', '#qbxml_map' );
  
// Filesystem access 
// Incoming and outgoing QBXML requests and responses are stored in this directory.
define( 'QBXML_FURI', 'qbxml' ); // URI to use for stashing qbxml requests in and out

// The files are stored with a prefix, the dynamic request id, and a suffix
define( 'QBXML_FEEDS_PREFIX', 'qbxml_import_' ); // default prefix for machine name of an importer for response tag (eg: TermsQueryRs) appended to end of name

// Iteration is enabled by default.
define( 'QBXML_CHUNK_SIZE', 100 );

// Default importer
define( 'QBXML_DEFAULT_IMPORTER', 'qbxml_feeds_import');

/* QBXML implements the following hook for any other modules to process incoming QBXML before
 * they are added to the queue.
 * Hooks should return 0 to allow processing to continue, anything else to interrupt processing
 */
define( 'HOOK_QBXML_PREPROCESS', 'qbxml_preprocess' );
define( 'HOOK_QBXML_FILEPROCESS', 'qbxml_fileprocess' );
define( 'HOOK_QBXML_IMPORTERS', 'qbxml_importers' );

// Rules events implemented
define( 'QBXML_EVENT_IN', 'qbxml_event_incoming_response' );


