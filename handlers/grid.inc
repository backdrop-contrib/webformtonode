<?php

/**
 * @file
 * Handler to map Webform Grid components to Node fields.
 */

/**
 * Maps Webform Grid values (Array of Arrays) to a formatted string.
 */
function webformtonode_grid_map($val, $node, $context) {
  $item = array();
  $component = $context['component'];

  // Grids are stored as: array('question_key' => 'answer_key')
  if (!is_array($val)) {
    return $item;
  }

  $output = array();

  // 1. Get the Human-Readable Labels for Rows and Columns.
  $rows = _webformtonode_grid_parse_options($component['extra']['options']);
  $cols = _webformtonode_grid_parse_options($component['extra']['questions']);

  // 2. Build the readable string.
  foreach ($val as $row_key => $col_key) {
    if (empty($col_key)) continue;

    $row_label = isset($rows[$row_key]) ? $rows[$row_key] : $row_key;
    $col_label = isset($cols[$col_key]) ? $cols[$col_key] : $col_key;

    $output[] = $row_label . ': ' . $col_label;
  }

  if (!empty($output)) {
    $item = array(
      'value' => implode("\n", $output),
      'format' => 'filtered_html',
    );
  }

  return $item;
}

/**
 * Helper to parse Webform's pipe-delimited strings (key|label).
 */
function _webformtonode_grid_parse_options($string) {
  $options = array();
  $lines = explode("\n", $string);
  foreach ($lines as $line) {
    $line = trim($line);
    if (empty($line)) continue;

    if (strpos($line, '|') !== FALSE) {
      list($key, $label) = explode('|', $line);
      $options[trim($key)] = trim($label);
    } else {
      $options[$line] = $line;
    }
  }
  return $options;
}