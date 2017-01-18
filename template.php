<?php
/**
 * Created by: hugh
 * Date: 12/21/16
 * Time: 8:20 AM
 */

function qbxml_preprocess(&$var, $hook) {
  $opcode = '';
  $var['theme_hook_suggestions'] = array();
  if (isset($var['qbxml_opcode'])) {
    $opcode = strtolower($var['qbxml_opcode']);
    $var['theme_hook_suggestions'][] = $opcode . '.qbxml';
    $var['theme_hook_suggestions'][] = 'qbxml__' . $opcode;
  }
  if (isset($var['qbxml_base'])) {
    $base = strtolower($var['qbxml_base']);
    $var['theme_hook_suggestions'][] = $base . '.qbxml';
    $var['theme_hook_suggestions'][] = 'qbxml__' . $base;
    if ($opcode) {
      $var['theme_hook_suggestions'][] = $base . $opcode . '.qbxml';
      $var['theme_hook_suggestions'][] = 'qbxml__' . $base . $opcode;
    }
  }
}