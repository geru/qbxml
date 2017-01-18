<?php
/**
 * @file
 * Hooks provided by the qbwc module.
 */

/**
 * @defgroup qbxml_api Quickbooks XML API
 * @{
 * Functions to interact or process Quickbooks XML requests as they are received.
 *
 * Quickbooks XML (QBXML) is the nerve-center of XML request and response
 * processing. Requests are generated on the Drupal site and sent through the
 * Quickbooks Webconnector module to a standalone Quickbooks or Quickbooks POS
 * site where they are processed and a response is sent back to Drupal.
 *
 * QBXML provides a theme hook to generate requests, and a Feeds-based importer
 * to map returned results into Drupal data structures. The requests and
 * responses are collated and kept together using a bookID concept compatibly
 * with the Quickbooks Webconnector and the BC_entity modules. This way, any
 * number of connections and accounting datasets can be maintained separately.
 *
 * QBXML maintains a queue of outgoing requests and sends them automatically
 * when an appropriate incoming request for requests is made. Requests are
 * submitted as QBXML renderable arrays via the
 * qbxml_action_add_xml_array_to_out_queue function
 *
 * If you imagine a QBXML outgoing request as a bunch of objects that have been
 * array-ified via a process like json_decode(json_encode($object), TRUE), then
 * you have the fundamental format concept. This concept is embellished with
 * various metadata elements to provide additional QBXML functionality. See the
 * admin operations form for an example.
 *
 * QBXML provides a theme hook for qbxml message formatting. Modules may tie
 * into this theme hook through the Drupal theme system. Themes can also be
 * defined by files name example.qbxml.tpl.php. The qbxml theme hook provides
 * theme suggestions based on a base code and a Mod, Add, or Query operation.
 *
 * QBXML provides several hooks to preprocess or process incoming QBXML responses. Additionally,
 * it provides a default Feeds-based importer to do this automatically. The default
 * importer looks for a Feeds importer named according to the incoming message content tags
 * and, if found, it will initiate the importer on the incoming data.
 *
 * @}
 */

/**
 * @addtogroup qbxml_api
 * @{
 * Identify a QBXML importer.
 *
 * This hook is used to allow externally defined QBXML import processors to
 * identify themselves to QBXML.
 *
 * When a request message is initially generated, it identifies the correct
 * import processor it wants to process the corresponding response. Import
 * processors must define themeselves with this hook in order to be found when
 * a response comes back
 *
 * @return The hook must return an array list of all of the import functions
 * defined within that module.
 **/
function hook_qbxml_importers() {
  return(array('mymodule_qbxml_import_function'));
}

/*
 * Inspect / preprocess the incoming QBXML data.
 *
 * This hook is invoked from the QBXML to allow external modules to process the
 * data however they wish. If another module wants to claim the data for itself,
 * it does so and returns something.
 *
 * External modules can also just inspect the data.
 *
 * @param $request
 *   the original request
 * @param $response
 *   the QBXML response
 *
 * @return
 *   Return Value: A non-empty value is returned to signify that the data has
 * been claimed and processed and no further processing should take place.
 **/
function hook_qbxml_preprocess($request, $response) {
  // do something to inspect the response and see if it has the relevant information
  // for this preprocessor function
  return (TRUE); // message was processed
  return (FALSE); // message was not processed
}

/*
 * File importer
 *
 * If no preprocessor function claims the data. Then, the default qbxml importer
 * will try to process it with the originally defined importer. Barring that
 * existence, it will use its own Feeds importer to try to import the message.
 * If an appropriate importer doesn't exist, then it punts, saves the file, and
 * fires the file importer hook to deal with the file.
 *
 * @param $filename
 *   the file sitting around waiting to be processed.
 *
 * @return
 *   Return Value: A non-empty value is returned to signify that the data has
 * been claimed and processed and the file can be deleted along with the request
 * information. If nothing is returned, then the file and the request will sit
 * around until something comes along to clean it all up. And, if nothing gets
 * implemented to do this cleanup, things will get quite full and messy.
 **/
function hook_qbxml_fileprocess($filename) {
  // do something to import or deal with this QBXML file
  return (TRUE); // file was processed
  return (FALSE); // file was not processed
}
