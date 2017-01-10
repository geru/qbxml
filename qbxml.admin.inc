<?php
// $Id$

/**
 * @file
 * Admin page callback for the qbxml module.
 */

require_once 'qbxml.inc';

/**
 * Builds and returns the admin settings form.
 */
function qbxml_admin_settings_form() {
  $form[QBXML_FURI] = array(
    '#title' => t('QBXML incoming and outgoing file directory (recommended to be private)'),
    '#type' => 'textfield',
    '#default_value' => variable_get(QBXML_FURI) ? variable_get(QBXML_FURI) : 'private://qbxml',
    '#description' => t('Set the directory path for storing QBXML requests and responses.'),
    '#required' => TRUE,
  );

  return system_settings_form($form);
}

/**
 * This is just a quick and dirty form to generate QBXML messages. Useful for
 * generating import queries.
 * Build and return the admin operations form.
 */
function qbxml_admin_operations_form() {

  $form = array();

  // Build list of QB lists
  $qb_list = array(
    "Account",
    "Class",
    "Customer",
    "CustomerMsg",
    "CustomerType",
    "DateDrivenTerms",
    "Employee",
    "ItemDiscount",
    "ItemFixedAsset",
    "ItemGroup",
    "ItemInventory",
    "ItemInventoryAssembly",
    "ItemNonInventory",
    "ItemOtherCharge",
    "ItemPayment",
    "ItemSalesTax",
    "ItemSalesTaxGroup",
    "ItemService",
    "ItemSubtotal",
    "JobType",
    "OtherName",
    "PaymentMethod",
    "PriceLevel",
    "SalesRep",
    "SalesTaxCode",
    "ShipMethod",
    "StandardTerms",
    "ToDo",
    "Vendor",
    "VendorType",
  );

  $form[QBXML_FORM_REQUEST_BASE] = array(
    '#title' => t('Quickbooks list'),
    '#type' => 'select',
    '#options' => $qb_list,
    '#description' => t('Select a list to import.'),
    '#size' => 15,
    '#multiple' => TRUE,
    '#required' => TRUE,
    '#attributes' => array('class' => array('views-exposed-widget', 'left')),
  );

  $form[QBXML_FORM_REQUEST_OPCODE] = array(
    '#title' => t('Operation'),
    '#type' => 'select',
    '#options' => array('Query', 'Mod', 'Add'),
    '#default_value' => 'Query',
    '#description' => t('Select an operation.'),
    '#required' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Send request to Quickbooks')
  );

  $form[QBXML_FORM_REQUEST_PREFIX . 'callback_feedsprefix'] = array(
    '#title' => t('Feeds importer prefix'),
    '#type' => 'textfield',
    '#default_value' => QBXML_FEEDS_PREFIX,
    '#required' => FALSE,
    '#description' => t('Feeds importer name = prefix . incomingresponse')
  );

  $form['bookid'] = array(
    '#title' => t('Book machine ID'),
    '#type' => 'textfield',
    '#default_value' => '1',
    '#description' => t('This is a book that will receive the data.'),
    '#element_validate' => array('element_validate_integer_positive'),
  );

  $form['activestatus'] = array(
    '#title' => t('Active status'),
    '#type' => 'radios',
    '#options' => array(
      0 => 'Not set',
      'ActiveOnly' => 'Active',
      'InactiveOnly' => 'Inactive',
      'All' => 'All'
    ),
    '#default_value' => 'All',
    '#description' => t('Active status of desired data.'),
  );

  $form['maxreturned'] = array(
    '#title' => t('Chunk size'),
    '#type' => 'textfield',
    '#description' => t('How many return elements at a time.'),
    '#element_validate' => array('element_validate_integer_positive'),
  );

  $form['timedatefrom'] = array(
    '#title' => t('Date from'),
    '#type' => 'textfield',
    '#description' => t('Date from (YYYY-MM-DD[ hh:mm:ss]'),
    // QBXML_THEME => 'qbxml_theme',
  );

  $form['timedateto'] = array(
    '#title' => t('Date to'),
    '#type' => 'textfield',
    '#description' => t('Date to (YYYY-MM-DD[ hh:mm:ss]'),
    // QBXML_THEME => 'qbxml_theme',
  );

  $form[QBXML_FORM_RAWQUERY] = array(
    '#title' => t('raw QBXML query'),
    '#type' => 'textarea',
    '#description' => t('Type properly formatted QBXML for the operation. NOT RECOMMENDED'),
  );

  // set $form['#theme']
  return $form;
}

/*
 * Callback to generate QBXML mapping (form_id_mapQBXML)
 */
function qbxml_admin_operations_form_mapQBXML() {
  return array();
}

/**
 * Submit function for input_simple_form().
 **/
function qbxml_admin_operations_form_submit($form_id, &$form_state) {
  drupal_set_message(t('The operation has been submitted.'));

  $requests = array();
  // create array to be rendered by format_xml_elements
  // multiple QBXMLMmsgsRq elements are allowed. We are assuming just a single one, but the Rule expects one or more in an array
  foreach ($form_state['values'][QBXML_FORM_REQUEST_BASE] as $listindex) {

    $qbcmd_base = $form_state['complete form'][QBXML_FORM_REQUEST_BASE]['#options'][$listindex];
    $qbcmd_opcode = $form_state['complete form'][QBXML_FORM_REQUEST_OPCODE]['#options'][$form_state['values'][QBXML_FORM_REQUEST_OPCODE]];
    $requests += qbxml_form_generate_request($form_state['values'], $qbcmd_base, $qbcmd_opcode);
  }
  // rules_invoke_component( 'qbxml_q_action_add_render_array_to_out_queue', $QBXMLMsgsRqs );
  qbxml_action_add_xml_array_to_out_queue($form_state['input']['bookid'], qbxml_generate_qbxmlmsgsrq($requests));
}
