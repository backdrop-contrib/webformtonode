<?php

/**
 * @file
 * Administration page callbacks for the Webform to Node module.
 */

/**
 * Form builder: Select which Webforms have "Copy to Node" functionality.
 */
function webformtonode_admin_select_form($form, &$form_state) {
  // Get all nodes of type 'webform'.
  $query = db_select('node', 'n')
    ->fields('n', array('nid', 'title'))
    ->condition('type', 'webform')
    ->execute();

  $options = array();
  foreach ($query as $row) {
    $options[$row->nid] = check_plain($row->title);
  }

  $form['webforms'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Enabled Webforms'),
    '#options' => $options,
    '#default_value' => config_get('webformtonode.settings', 'webforms') ?: array(),
    '#description' => t('Select the Webforms that should allow submissions to be converted into nodes.'),
  );

  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save Enabled Webforms'),
  );

  return $form;
}

/**
 * Submit handler for webformtonode_admin_select_form.
 */
function webformtonode_admin_select_form_submit($form, &$form_state) {
  $config = config('webformtonode.settings');
  // Filter out unselected checkboxes (0 values).
  $enabled = array_filter($form_state['values']['webforms']);
  $config->set('webforms', $enabled);
  $config->save();

  backdrop_set_message(t('Enabled webforms updated.'));
}

/**
 * Page callback: Lists enabled webforms and provides links to map them.
 */
function webformtonode_admin_manage_form($form, &$form_state) {
  $enabled_nids = config_get('webformtonode.settings', 'webforms');

  if (empty($enabled_nids)) {
    $form['help'] = array(
      '#markup' => '<p>' . t('No webforms enabled. Please <a href="@url">select webforms</a> first.',
      array('@url' => url('admin/config/webformtonode/settings/select_webforms'))) . '</p>',
    );
    return $form;
  }

  $header = array(t('Webform Title'), t('Operations'));
  $rows = array();

  $nodes = node_load_multiple($enabled_nids);
  foreach ($nodes as $node) {
    $rows[] = array(
      check_plain($node->title),
      l(t('Configure Mapping'), 'admin/config/webformtonode/settings/manage_webforms/fields/' . $node->nid),
    );
  }

  $form['table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No enabled webforms found.'),
  );

  return $form;
}

/**
 * Form builder: Map Webform components to Node fields.
 */
function webformtonode_admin_fields_list_form($form, &$form_state, $webform_nid) {
  $webform_node = node_load($webform_nid);
  $config = config('webformtonode.settings');

  // 1. Target Content Type Selection
  $node_types = node_type_get_names();
  $target_type = $config->get('mapping_' . $webform_nid . '_type');

  $form['target_type'] = array(
    '#type' => 'select',
    '#title' => t('Target Content Type'),
    '#options' => array('' => t('- Select -')) + $node_types,
    '#default_value' => $target_type,
    '#description' => t('Which content type should submissions create?'),
    '#ajax' => array(
      'callback' => 'webformtonode_admin_fields_refresh',
      'wrapper' => 'mapping-table-wrapper',
    ),
  );

  // 2. Mapping Table Wrapper (for AJAX)
  $form['mapping_wrapper'] = array(
    '#prefix' => '<div id="mapping-table-wrapper">',
    '#suffix' => '</div>',
  );

  // Only show the mapping grid if a target type is selected
  $selected_type = isset($form_state['values']['target_type']) ? $form_state['values']['target_type'] : $target_type;

  if ($selected_type) {
    $targets = webformtonode_get_target_fields($selected_type);
    $current_mapping = $config->get('mapping_' . $webform_nid . '_fields') ?: array();

    $form['mapping_wrapper']['mapping'] = array(
      '#tree' => TRUE,
      '#theme' => 'table',
      '#header' => array(t('Webform Component (Source)'), t('Node Field (Target)')),
      '#rows' => array(),
    );

    foreach ($webform_node->webform['components'] as $cid => $component) {
      $key = $component['form_key'];

      $form['mapping_wrapper']['mapping'][$key] = array(
        '#type' => 'select',
        '#options' => array('' => t('- Skip Field -')) + $targets,
        '#default_value' => isset($current_mapping[$key]) ? $current_mapping[$key] : '',
      );

      $form['mapping_wrapper']['mapping']['#rows'][] = array(
        'data' => array(
          $component['name'] . ' <small>(' . $key . ')</small>',
          array('data' => &$form['mapping_wrapper']['mapping'][$key]),
        ),
      );
    }
  }

  $form['webform_nid'] = array('#type' => 'value', '#value' => $webform_nid);

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save Mapping'),
  );

  return $form;
}

/**
 * AJAX Callback to refresh the mapping table when Content Type changes.
 */
function webformtonode_admin_fields_refresh($form, $form_state) {
  return $form['mapping_wrapper'];
}

/**
 * Submit handler for the mapping form.
 */
function webformtonode_admin_fields_list_form_submit($form, &$form_state) {
  $nid = $form_state['values']['webform_nid'];
  $config = config('webformtonode.settings');

  $config->set('mapping_' . $nid . '_type', $form_state['values']['target_type']);
  $config->set('mapping_' . $nid . '_fields', $form_state['values']['mapping']);
  $config->save();

  backdrop_set_message(t('Mapping configuration for %title saved.', array('%title' => $form['#node']->title)));
}