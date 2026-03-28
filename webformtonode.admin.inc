<?php

/**
 * @file
 * Administration page callbacks for the Webform to Node module.
 */

/**
 * Page callback: The unified landing page (Command Center).
 */
/**
 * Page callback: The unified landing page (Command Center).
 */
function webformtonode_admin_settings_landing() {
  $build = array();

  // 1. Standard Admin Menu Block.
  // This shows the "Manage webform selection" link at the top.
  module_load_include('inc', 'system', 'system.admin');
  $build['menu_list'] = array(
    '#markup' => system_admin_menu_block_page(),
  );

  $config = config('webformtonode.settings');
  $enabled_nids = $config->get('webforms') ?: array();

  // 2. The Dashboard / Status Table.
  if (!empty($enabled_nids)) {
    $header = array(t('Webform Title'), t('Status'), t('Operations'));
    $rows = array();

    // Load all enabled nodes at once for efficiency.
    $nodes = node_load_multiple($enabled_nids);

    foreach ($nodes as $node) {
      $mapping = $config->get('mapping_' . $node->nid . '_fields');
      $status_text = !empty($mapping) ? '✅ ' . t('Mapped') : '⚠️ ' . t('Pending');

      // Build Quick Links for the Title Column.
      // We add a destination back to this page for the Edit link.
      $quick_links = array();
      $quick_links[] = l(t('Edit'), 'node/' . $node->nid . '/edit', array(
        'query' => array('destination' => 'admin/config/webformtonode/settings'),
      ));
      $quick_links[] = l(t('Submissions'), 'node/' . $node->nid . '/webform-results');

      // Compose the Title Markup with a small grayed-out sub-menu.
      $title_markup = '<strong>' . check_plain($node->title) . '</strong>';
      $title_markup .= ' <small style="margin-left: 10px; color: #666;">(' . implode(' | ', $quick_links) . ')</small>';

      // Define the Operations URL.
      $config_url = 'admin/config/webformtonode/settings/manage_webforms/fields/' . $node->nid;

      $rows[] = array(
        'data' => array(
          $title_markup,
          $status_text,
          l(t('Configure'), $config_url, array(
            'attributes' => array('class' => array('button', 'button-secondary')),
          )),
        ),
      );
    }

    $build['status_report'] = array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
      '#prefix' => '<h3>' . t('Current Configuration Status') . '</h3>',
    );
  }
  else {
    // Helpful empty state message.
    $build['status_report'] = array(
      '#markup' => '<div class="messages warning">' .
        t('No webforms enabled. Please use <a href="@url">Manage webform selection</a> to get started.',
        array('@url' => url('admin/config/webformtonode/settings/select_webforms'))) .
        '</div>',
    );
  }

  return $build;
}

/**
 * Form builder: Select which Webforms are enabled for promotion.
 */
function webformtonode_admin_select_form($form, &$form_state) {
  backdrop_set_title(t('Manage webform selection'));

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
    '#description' => t('Check the webforms you want to enable. <strong>Note:</strong> Unchecking a webform and confirming on the next screen will permanently delete its field mapping configuration.'),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  $form['actions']['cancel'] = array(
    '#markup' => l(t('Cancel'), 'admin/config/webformtonode/settings', array(
      'attributes' => array('class' => array('button')),
    )),
  );

  return $form;
}

/**
 * Submit handler for webformtonode_admin_select_form.
 */
/**
 * Submit handler for webformtonode_admin_select_form.
 */
function webformtonode_admin_select_form_submit($form, &$form_state) {
  $config = config('webformtonode.settings');
  $old_enabled = $config->get('webforms') ?: array();

  // FIX: array_filter() without a callback removes all "falsy" values (0, "", NULL, etc.)
  $new_enabled = array_values(array_filter($form_state['values']['webforms']));

  // Check for removals for the confirmation redirect...
  $removed = array_diff($old_enabled, $new_enabled);

  if (!empty($removed)) {
    $form_state['redirect'] = array(
      'admin/config/webformtonode/settings/select_webforms/confirm',
      array(
        'query' => array(
          'removed' => implode(',', $removed),
          'new_list' => implode(',', $new_enabled),
        ),
      ),
    );
  }
  else {
    // Save the clean, filtered array.
    $config->set('webforms', $new_enabled);
    $config->save();

    backdrop_set_message(t('Enabled webforms updated.'));
    $form_state['redirect'] = 'admin/config/webformtonode/settings';
  }
}

/**
 * Form builder: Safety confirmation for removing mappings.
 */
function webformtonode_admin_select_confirm_form($form, &$form_state) {
  $removed_nids = explode(',', $_GET['removed'] ?? '');
  $new_list = explode(',', $_GET['new_list'] ?? '');

  $titles = array();
  foreach ($removed_nids as $nid) {
    if ($node = node_load($nid)) {
      $titles[] = '<strong>' . check_plain($node->title) . '</strong>';
    }
  }

  $form['removed_nids'] = array('#type' => 'value', '#value' => $removed_nids);
  $form['new_list'] = array('#type' => 'value', '#value' => $new_list);

  $description = t('Are you sure you want to disable these webforms? This will permanently delete all field mappings for:') . '<ul><li>' . implode('</li><li>', $titles) . '</li></ul>';

  return confirm_form(
    $form,
    t('Confirm removal of webform configurations'),
    'admin/config/webformtonode/settings/select_webforms',
    $description,
    t('Delete Configurations'),
    t('Cancel')
  );
}

/**
 * Submit handler for the safety confirmation.
 */
function webformtonode_admin_select_confirm_form_submit($form, &$form_state) {
  $config = config('webformtonode.settings');
  $removed_nids = $form_state['values']['removed_nids'];
  $new_list = $form_state['values']['new_list'];

  foreach ($removed_nids as $nid) {
    $config->clear('mapping_' . $nid . '_type');
    $config->clear('mapping_' . $nid . '_fields');
    $config->clear('node_status_' . $nid);
  }

  $config->set('webforms', $new_list);
  $config->save();

  backdrop_set_message(t('Configurations cleared and webforms disabled.'));
  $form_state['redirect'] = 'admin/config/webformtonode/settings';
}

/**
 * Form builder: Configure a specific webform's field mapping.
 */
function webformtonode_admin_fields_list_form($form, &$form_state, $webform_nid) {
  $webform_node = node_load($webform_nid);
  $config = config('webformtonode.settings');

  backdrop_set_title(t('Configure: @title', array('@title' => $webform_node->title)));

  $form['webform_nid'] = array('#type' => 'value', '#value' => $webform_nid);

  // 1. Target Content Type.
  $target_type = $config->get('mapping_' . $webform_nid . '_type');
  $node_types = node_type_get_names();

  $form['target_type'] = array(
    '#type' => 'select',
    '#title' => t('Target Content Type'),
    '#options' => array('' => t('- Select -')) + $node_types,
    '#default_value' => $target_type,
    '#description' => t('Select the content type that submissions from this webform will create.'),
  );

  // 2. Field Mapping Table (Shows only if a target is selected).
  if ($target_type) {
    $targets = webformtonode_get_target_fields($target_type);
    $current_mapping = $config->get('mapping_' . $webform_nid . '_fields') ?: array();

    $form['mapping'] = array(
      '#tree' => TRUE,
      '#theme' => 'table',
      '#header' => array(t('Webform Component'), t('Node Field')),
      '#rows' => array(),
      '#prefix' => '<h3>' . t('Field Mapping') . '</h3>',
    );

    foreach ($webform_node->webform['components'] as $cid => $component) {
      $key = $component['form_key'];
      $form['mapping'][$key] = array(
        '#type' => 'select',
        '#options' => array('' => t('- Skip Field -')) + $targets,
        '#default_value' => isset($current_mapping[$key]) ? $current_mapping[$key] : '',
      );

      $form['mapping']['#rows'][] = array(
        $component['name'] . ' <small>(' . $key . ')</small>',
        array('data' => &$form['mapping'][$key]),
      );
    }

    // 3. Additional Processing Options.
    $form['options'] = array(
      '#type' => 'fieldset',
      '#title' => t('Additional Processing Options'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );

    $form['options']['node_status'] = array(
      '#type' => 'radios',
      '#title' => t('New Node Status'),
      '#options' => array(1 => t('Published'), 0 => t('Unpublished (Draft)')),
      '#default_value' => $config->get('node_status_' . $webform_nid) ?? 1,
    );

    $form['options']['validation_note'] = array(
      '#markup' => '<div class="messages help">' . t('<strong>Note:</strong> When a webform submission is saved as a node, if the content type has a required field, standard field validation will not occur. If the webform does not contain data for a required field, that field will be left blank in the created node.') . '</div>',
    );
  }

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save Configuration'));
  $form['actions']['cancel'] = array(
    '#markup' => l(t('Cancel'), 'admin/config/webformtonode/settings', array(
      'attributes' => array('class' => array('button')),
    )),
  );

  return $form;
}

/**
 * Submit handler for the mapping configuration form.
 */
function webformtonode_admin_fields_list_form_submit($form, &$form_state) {
  $nid = $form_state['values']['webform_nid'];
  $config = config('webformtonode.settings');

  $config->set('mapping_' . $nid . '_type', $form_state['values']['target_type']);
  $config->set('mapping_' . $nid . '_fields', $form_state['values']['mapping'] ?? array());
  $config->set('node_status_' . $nid, (int) ($form_state['values']['node_status'] ?? 1));

  $config->save();
  backdrop_set_message(t('Configuration for webform %nid saved.', array('%nid' => $nid)));
}