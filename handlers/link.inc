<?php

/**
 * @file
 * Handler to map Webform Link components to Node fields.
 */

/**
 * Maps Webform link values to Node link or text fields.
 */
function webformtonode_link_map($val, $node, $context) {
  $item = array();

  // Link components usually provide a URL string.
  // If the Webform Link component allows titles, $val might be an array.
  $url = is_array($val) ? ($val['url'] ?? reset($val)) : $val;
  $title = is_array($val) && isset($val['title']) ? $val['title'] : '';

  if (empty($url)) {
    return $item;
  }

  // Check the target field type.
  $field_type = $context['field_info']['type'];

  if ($field_type == 'link_field') {
    // Backdrop Link module expects 'url' and optionally 'title'.
    $item = array(
      'url' => $url,
      'title' => $title,
    );
  }
  else {
    // Fallback for standard text fields.
    $item = array('value' => $url);
  }

  return $item;
}