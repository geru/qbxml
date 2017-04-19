<?php
$tid = empty($value['und'][0]['tid']) ? NULL : $value['und'][0]['tid'];
print( ($tid !== NULL) && ($term = taxonomy_term_load($tid))  
  ? qbxml_string_to_camelterm($term->name)
  : '' );