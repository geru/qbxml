<?php
/**
 * Created by: hugh
 * Date: 12/21/16
 * Time: 8:20 AM
 */

/**
 * Set deep array with conditional against mask
 * @param $outputarray
 * @param $formatkey
 * @param $value
 * @param $maskkey
 * @param null $maskarray
 */
function _bc_conditional_set_deeparray( &$outputarray, $formatkey, $value, $maskkey, $maskarray=NULL ) {
  if ($maskarray) {
    if (isset($maskarray[$maskkey])) {
      if( !$maskarray[$maskkey] ) {
        qbxml_set_deep_array_element($outputarray, $formatkey, $value);
      } else {
        // todo: check mask conditions before setting
        qbxml_set_deep_array_element($outputarray, $formatkey, $value);
      }
    }
  }
  else {
    qbxml_set_deep_array_element($outputarray, $formatkey, $value);
  }
}

/**
 * Generate an array that can be transformed directly into XML
 * @param $values - array of values
 * @param $fmt - formatting information
 * @param $mask - mask to trim output array
 * @return array - XML-renderable array
 * @throws \Exception
 */
function _bc_theme_generate_output_array( $values, $fmt, $mask=NULL ) {
  $outputarray = array();
  $bcrefs = NULL;
  $data = _bc_unflatten_array($values);
  foreach( $fmt as $fmtset ) {
    foreach( $fmtset as $fmttype => $fmtfields ) {
      switch ($fmttype) {
        // special case: unlimited of these are attached to a beancount. This figures out the type and lists the ref extid wrapped with appropriate tags
        case 'bc ref':
          foreach ($fmtfields as $name => $themeinfo) {
            if ($bcrefs === NULL) {
              $bcids = array();
              if( !empty($values['bc_bcref']['und'])) {
                foreach ($values['bc_bcref']['und'] as $bcref) {
                  if (!empty($bcref['target_id'])) {
                    $bcids[] = $bcref['target_id'];
                  }
                }
                $bcrefs = bc_load_multiple($bcids);
              } else {
                $bcrefs = array();
              }
            }
            if( $bcrefs ) {
              $targetsubtype = _bc_list_subtype(NULL, $name, 0);
              foreach ($bcrefs as $bcref) {
                if ($bcref->subtype == $targetsubtype) {
                  $vals = explode('::', $bcref->extid);
                  if (!empty($vals[1])) {
                    $value = isset($themeinfo['theme'])
                      ? theme($themeinfo['theme'], array( 'value' => $vals[1] ) )
                      : $vals[1];
                    $refterm = isset($themeinfo['key']) ? $themeinfo['key'] : $name;
                    $formatkey = array( $refterm . 'Ref', 'ListID');
                    _bc_conditional_set_deeparray( $outputarray, $formatkey, $value, $name, $mask );
                  }
                  break;
                }
              }
            }
          }
          break;
        // data is for auxiliary information stored in data blob
        case 'data':
          foreach( $fmtfields as $name => $themeinfo ) {
            if (isset($data[$name])) {
              $value = !empty($themeinfo['theme']) ? theme($themeinfo['theme'], array('value' => $data[$name])) : $data[$name];
              $formatkey = isset($themeinfo['key']) ? $themeinfo['key'] : $name;
              _bc_conditional_set_deeparray( $outputarray, $formatkey, $value, $name, $mask );
              break;
            }
          }
          break;
        // any standard property or field
        case 'value':
          foreach( $fmtfields as $name => $themeinfo ) {
            if (isset($values[$name])) {
              $value = !empty($themeinfo['theme']) ? theme($themeinfo['theme'], array('value' => $values[$name])) : $values[$name];
              $formatkey =  isset($themeinfo['key']) ? $themeinfo['key'] : $name;
              _bc_conditional_set_deeparray( $outputarray, $formatkey, $value, $name, $mask );
            }
          }

      }
    }
  }
  return $outputarray;
}

/**
 * Implements theme_preprocess()
 * @param $var
 * @param $hook
 */
function qbxml_preprocess(&$var, $hook) {
  $var['theme_hook_suggestions'] = array();
  $bases = array();
  if( isset($var['qbxml_opcode']) ) {
    $opcode = strtolower($var['qbxml_opcode']);
    $var['theme_hook_suggestions'][] = $opcode . '.qbxml';
    $var['theme_hook_suggestions'][] = 'qbxml_' . $opcode;
  } else {
    $opcode = '';
  }
  if (isset($var['qbxml_base'])) {
    $bases[] = strtolower($var['qbxml_base']);
  }
  if (isset($var['qbxml_type'])) {
    $bases[] = strtolower($var['qbxml_type']);
    if( isset($var['qbxml_base']) ) {
      $bases[] = strtolower($var['qbxml_base']) . strtolower($var['qbxml_type']); 
    }
  }
  foreach( $bases as $base ) {  
    $var['theme_hook_suggestions'][] = $base . '.qbxml';
    $var['theme_hook_suggestions'][] = 'qbxml_' . $base;
  }
  if( $opcode ) {
    foreach ($bases as $base) {
      $var['theme_hook_suggestions'][] = $base . $opcode . '.qbxml';
      $var['theme_hook_suggestions'][] = 'qbxml_' . $base . $opcode;
    }
  }
}