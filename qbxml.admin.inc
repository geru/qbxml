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
    '#default_value' => variable_get(QBXML_FURI, 'private://qbxml'),
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

  $qb_list = array(
    "Party" => array(
      "Customer",
      "Employee",
      "OtherName",
      "Vendor",
      "Entity",
    ),
    "Accounts" => array(
      "Account",
    ),
    "Item" => array(
      "ItemService",
      "ItemNonInventory",
      "ItemOtherCharge",
      "ItemInventory",
      "ItemInventoryAssembly",
      "ItemFixedAsset",
      "ItemSubtotal",
      "ItemDiscount",
      "ItemPayment",
      "ItemSalesTax",
      "ItemSalesTaxGroup",
      "ItemGroup",
      "Item",
    ),
    "List" => array(
      "Class",
      "CustomerMsg",
      "CustomerType",
      "StandardTerms",
      "DateDrivenTerms",
      "Terms",
      "VendorType",
      "JobType",
      "PaymentMethod",
      "ShipMethod",
      "SalesTaxCode",
      "PriceLevel",
      "BillingRate",
    ),
    "Data set" => array(
      "SalesRep",
      "ToDo",
    ),
    "Company" => array(
      "Host",
      "Company",
      "CompanyActivity",
      "Preferences",
      "Template",
      "Currency",
      "InventorySite",
      "DataExtDef",
    ),
    "Other" => array(
      "AccountTaxLineInfo",
      "Lead",
      "Vehicle",
      "UnitOfMeasureSet",
      "ItemSites",
      "BarCode",
    ),
    "Tally" => array(
      "SalesTaxReturn",
      "SalesTaxReturnLine",
      "Invoice",
      "Estimate",
      "SalesOrder",
      "SalesReceipt",
      "CreditMemo",
      "PurchaseOrder",
      "Bill",
      "ItemReceipt",
      "VendorCredit",
      "BillToPay",
      "SalesTaxPayable",
      "BuildAssembly",
      "ItemAssembliesCanBuild",
      "VehicleMileage",
      "TransferInventory",
      "InventoryAdjustment",
    ),
    "Employee" => array(
      "PayrollItemWage",
      "PayrollItemNonWage",
      "WorkersCompCode",
      "Form1099CategoryAccountMapping",
      "PayrollLastPeriod",
      "TimeTracking",
    ),
    "Transaction" => array(
      "ReceivePayment",
      "ReceivePaymentToDeposit",
      "Charge",
      "BillPaymentCheck",
      "BillPaymentCreditCard",
      "SalesTaxPaymentCheck",
      "Check",
      "CreditCardCharge",
      "CreditCardCredit",
      "JournalEntry",
      "Deposit",
      "Transaction",
      "ARRefundCreditCard",
      "Transfer",
    ),
    "Report" => array(
      "GeneralSummaryReport",
      "JobReport",
      "TimeReport",
      "AgingReport",
      "BudgetSummaryReport",
      "GeneralDetailReport",
      "CustomDetailReport",
      "CustomSummaryReport",
      "PayrollDetailReport",
      "PayrollSummaryReport",
    ),
/*    "Sync" => array(
      "ListDeleted",
      "TxnDeleted",
      "DataEventRecoveryInfo",
    ), 
*/
  );

  foreach( $qb_list as $list => $listoptions ) {
    $form[QBXML_FORM_REQUEST_BASE . $list] = array(
      '#title' => t($list),
      '#type' => 'select',
      '#options' => $listoptions,
      '#description' => t('Select list(s) to import.'),
      '#size' => count($listoptions),
      '#multiple' => TRUE,
      '#required' => FALSE,
      '#attributes' => array('class' => array('views-exposed-widget', 'left')),
    );
  }

  $form[QBXML_FORM_REQUEST_OPCODE] = array(
    '#title' => t('Operation'),
    '#type' => 'select',
    '#options' => array('Query', 'Mod', 'Add'),
    '#default_value' => 'Query',
    '#description' => t('Select an operation.'),
    '#required' => TRUE,
  );

  $form['advanced'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form['advanced']['bookid'] = array(
    '#title' => t('Book machine ID'),
    '#type' => 'textfield',
    '#description' => t('This is a book that will receive the data.'),
    '#default_value' => function_exists('_bc_user_default_bookid') ? _bc_user_default_bookid() : 1,
    '#element_validate' => array('element_validate_integer_positive'),
  );

  $form['advanced'][QBXML_FORM_REQUEST_PREFIX . 'callback_feedsprefix'] = array(
    '#title' => t('Feeds importer prefix'),
    '#type' => 'textfield',
    '#default_value' => QBXML_FEEDS_PREFIX,
    '#required' => FALSE,
    '#description' => t('Feeds importer name = prefix . incomingresponse')
  );

  $form['advanced']['activestatus'] = array(
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

  $form['advanced']['maxreturned'] = array(
    '#title' => t('Chunk size'),
    '#type' => 'textfield',
    '#default_value' => '50', // seems to be a nice safe number
    '#description' => t('How many return elements at a time.'),
    '#element_validate' => array('element_validate_integer_positive'),
  );

  $form['advanced']['timedatefrom'] = array(
    '#title' => t('Date from'),
    '#type' => 'textfield',
    '#description' => t('Date from (YYYY-MM-DD[ hh:mm:ss]'),
  );

  $form['advanced']['timedateto'] = array(
    '#title' => t('Date to'),
    '#type' => 'textfield',
    '#description' => t('Date to (YYYY-MM-DD[ hh:mm:ss]'),
  );

  $form['advanced'][QBXML_FORM_RAWQUERY] = array(
    '#title' => t('raw QBXML query'),
    '#type' => 'textarea',
    '#description' => t('Type properly formatted QBXML for the operation. NOT RECOMMENDED'),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Send request to Quickbooks')
  );
  
  $form[QBXML_FORM_REQUEST_PREFIX . 'callback_importtermmapper'] = array(
    '#type' => 'textfield',
    '#access' => FALSE,
    '#value' => 'bc_importtermmapper' );

  return $form;
}

/*
 * Implements hook_form_validate()
 */
function qbxml_admin_operations_form_validate($form_id, &$form_state) {
  $bookid = $form_state['values']['bookid'];
  if (function_exists('_bc_user_accesses_book') && !_bc_user_accesses_book($bookid)) {
    form_set_error('QBXML', t('You need access to bookID :bookid. See your site administrator.', array(':bookid' => $bookid)));
  }
}

/**
 * Submit function for input_simple_form().
 **/
function qbxml_admin_operations_form_submit($form_id, &$form_state) {
  drupal_set_message(t('The operation has been submitted.'));

  // create array to be rendered by format_xml_elements
  // multiple QBXMLMmsgsRq elements are allowed. However, be careful about submitting more than one.
  // Quickbooks runs out of memory.
  foreach( preg_grep('/^' . QBXML_FORM_REQUEST_BASE . '.*/', array_keys($form_state['values'])) as $listname ) {
    foreach ($form_state['values'][$listname] as $listindex) {
      $request = array();

      $qbcmd_base = $form_state['complete form'][$listname]['#options'][$listindex];
      $qbcmd_opcode = $form_state['complete form'][QBXML_FORM_REQUEST_OPCODE]['#options'][$form_state['values'][QBXML_FORM_REQUEST_OPCODE]];
      $request[] = qbxml_form_generate_request($form_state['values'], $qbcmd_base, $qbcmd_opcode);

      qbxml_action_add_xml_array_to_out_queue($form_state['values']['bookid'], qbxml_generate_qbxmlmsgsrq($request));
    }
  }
}
