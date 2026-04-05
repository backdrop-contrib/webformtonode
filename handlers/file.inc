<?php

/**
 * @file
 * Handler to map Webform File components to Node fields.
 */

function webformtonode_file_map($val, $node, $context) {
  $item = array();

  // We use the helper function already in the .module file.
  $new_fid = _webformtonode_copy_file($val, $node, $context['node_target']);

  if ($new_fid) {
    $item = array(
      'fid' => $new_fid,
      'display' => 1,
    );
  }

  return $item;
}