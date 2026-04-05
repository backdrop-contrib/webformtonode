<?php

/**
 * @file
 * Handler to map Webform Select components to Node fields.
 */

/**
 * Maps Webform select values to Node list fields.
 */
function webformtonode_select_map($val, $node, $context) {
  $item = array();
  $component = $context['component'];

  // Select components can be single values or arrays (if multiple is checked).
  // We ensure we are working with an array for consistency.
  $values = is_array($val) ? $val : array($val);

  foreach ($values as $value) {
    if ($value === '' || $value === NULL) {
      continue;
    }

    // Logic: In Backdrop, List fields (text/int) usually just need the key.
    // However, if the target is a simple Text field, the user might want the Label.
    $field_type = $context['field_info']['type'];

    if ($field_type == 'text' || $field_type == 'text_long') {
      // Find the label for this key from the component's options.
      $lines = explode("\n", $component['extra']['items']);
      $label = $value; // Default to key if label isn't found.
      foreach ($lines as $line) {
        if (strpos($line, '|') !== FALSE) {
          list($key, $opt_label) = explode('|', $line);
          if (trim($key) == $value) {
            $label = trim($opt_label);
            break;
          }
        }
      }
      $item = array('value' => $label);
    }
    else {
      // Default: Map the raw key to the target field (Best for List/Boolean fields).
      $item = array('value' => $value);
    }
  }

  return $item;
}