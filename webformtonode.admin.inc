<?php

/**
 * @file
 * Administration page callbacks for the Webform to Node module.
 */

/**
 * Page callback: The unified landing page (Command Center).
 */
function webformtonode_admin_settings_landing() {
  $build = array();
  module_load_include('inc', 'system', 'system.admin');
  $build['menu_list'] = array('#markup' => system_admin_menu_block_page());

  $config = config('webformtonode.settings');
  $enabled_nids = $config->get('webforms') ?: array();

  if (!empty($enabled_nids)) {
    $header = array(t('Webform Title'), t('Status'), t('Operations'));
    $rows = array();
    $nodes = node_load_multiple($enabled_nids);

    foreach ($nodes as $node) {
      $mapping = $config->get('mapping_' . $node->nid . '_fields');
      $status_text = !empty($mapping) ? '✅ ' . t('Mapped') : '⚠️ ' . t('Pending');

      $quick_links = array();
      $quick_links[] = l(t('Edit'), 'node/' . $node->nid . '/edit', array('query' => array('destination' => 'admin/config/webformtonode/settings')));
      $quick_links[] = l(t('Submissions'), 'node/' . $node->nid . '/webform-results');

      $title_markup = '<strong>' . check_plain($node->title) . '</strong>';
      $title_markup .= ' <small style="margin-left: 10px; color: #666;">(' . implode(' | ', $quick_links) . ')</small>';

      $rows[] = array(
        'data' => array(
          $title_markup,
          $status_text,
          l(t('Configure'), 'admin/config/webformtonode/settings/manage_webforms/fields/' . $node->nid, array('attributes' => array('class' => array('button', 'button-secondary')))),
        ),
      );
    }
    $build['status_report'] = array('#theme' => 'table', '#header' => $header, '#rows' => $rows, '#prefix' => '<h3>' . t('Current Configuration Status') . '</h3>');
  } else {
    $build['status_report'] = array('#markup' => '<div class="messages warning">' . t('No webforms enabled. Use <a href="@url">Manage webform selection</a>.', array('@url' => url('admin/config/webformtonode/settings/select_webforms'))) . '</div>');
  }
  return $build;
}

/**
 * Form builder: Select which Webforms are enabled.
 */
function webformtonode_admin_select_form($form, &$form_state) {
  backdrop_set_title(t('Manage webform selection'));
  $query = db_select('node', 'n')->fields('n', array('nid', 'title'))->condition('type', 'webform')->execute();
  $options = array();
  foreach ($query as $row) { $options[$row->nid] = check_plain($row->title); }

  $form['webforms'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Enabled Webforms'),
    '#options' => $options,
    '#default_value' => config_get('webformtonode.settings', 'webforms') ?: array(),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save'));
  $form['actions']['cancel'] = array('#markup' => l(t('Cancel'), 'admin/config/webformtonode/settings', array('attributes' => array('class' => array('button')))));
  return $form;
}

function webformtonode_admin_select_form_submit($form, &$form_state) {
  $config = config('webformtonode.settings');
  $old_enabled = $config->get('webforms') ?: array();
  $new_enabled = array_values(array_filter($form_state['values']['webforms']));

  $removed = array_diff($old_enabled, $new_enabled);
  if (!empty($removed)) {
    $form_state['redirect'] = array('admin/config/webformtonode/settings/select_webforms/confirm', array('query' => array('removed' => implode(',', $removed), 'new_list' => implode(',', $new_enabled))));
  } else {
    $config->set('webforms', $new_enabled)->save();
    $form_state['redirect'] = 'admin/config/webformtonode/settings';
  }
}

/**
 * Form builder: Safety confirmation for removing mappings.
 */
function webformtonode_admin_select_confirm_form($form, &$form_state) {
  $removed_nids = explode(',', $_GET['removed'] ?? '');
  $titles = array();
  foreach ($removed_nids as $nid) { if ($node = node_load($nid)) { $titles[] = '<strong>' . check_plain($node->title) . '</strong>'; } }

  $form['removed_nids'] = array('#type' => 'value', '#value' => $removed_nids);
  $form['new_list'] = array('#type' => 'value', '#value' => explode(',', $_GET['new_list'] ?? ''));

  $desc = t('Permanently delete mappings for:') . '<ul><li>' . implode('</li><li>', $titles) . '</li></ul>';
  return confirm_form($form, t('Confirm removal?'), 'admin/config/webformtonode/settings/select_webforms', $desc, t('Delete'), t('Cancel'));
}

function webformtonode_admin_select_confirm_form_submit($form, &$form_state) {
  $config = config('webformtonode.settings');
  foreach ($form_state['values']['removed_nids'] as $nid) {
    $config->clear('mapping_' . $nid . '_type')->clear('mapping_' . $nid . '_fields')->clear('node_status_' . $nid);
  }
  $config->set('webforms', $form_state['values']['new_list'])->save();
  $form_state['redirect'] = 'admin/config/webformtonode/settings';
}

/**
 * Form builder: Configure a specific webform's field mapping.
 */
function webformtonode_admin_fields_list_form($form, &$form_state, $webform_nid) {
  $webform_node = node_load($webform_nid);
  $config = config('webformtonode.settings');
  backdrop_set_title(t('Configure: @title', array('@title' => $webform_node->title)));

  $form['back_link'] = array(
    '#markup' => '<div style="margin-bottom: 20px;">' . l(t('« Back to Settings'), 'admin/config/webformtonode/settings') . '</div>',
    '#weight' => -100,
  );

  $form['webform_nid'] = array('#type' => 'value', '#value' => $webform_nid);
  $target_type = $config->get('mapping_' . $webform_nid . '_type');

  $form['target_type'] = array(
    '#type' => 'select',
    '#title' => t('Target Content Type'),
    '#options' => array('' => t('- Select -')) + node_type_get_names(),
    '#default_value' => $target_type,
    '#description' => t('Select the content type. Change and save to update field options.'),
  );

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

    foreach ($webform_node->webform['components'] as $cid => $comp) {
      if ($comp['type'] == 'fieldset') continue;
      $key = $comp['form_key'];
      $form['mapping'][$key] = array(
        '#type' => 'select',
        '#options' => array('' => t('- Skip -')) + $targets,
        '#default_value' => isset($current_mapping[$key]) ? $current_mapping[$key] : '',
      );
      $form['mapping']['#rows'][] = array($comp['name'] . ' <small>(' . $key . ')</small>', array('data' => &$form['mapping'][$key]));
    }

    $form['options'] = array('#type' => 'fieldset', '#title' => t('Options'), '#collapsible' => TRUE, '#collapsed' => TRUE);
    $form['options']['node_status'] = array(
      '#type' => 'radios',
      '#title' => t('New Node Status'),
      '#options' => array(1 => t('Published'), 0 => t('Unpublished')),
      '#default_value' => $config->get('node_status_' . $webform_nid) ?? 1,
    );
  }

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save Configuration'));
  return $form;
}

function webformtonode_admin_fields_list_form_submit($form, &$form_state) {
  $nid = $form_state['values']['webform_nid'];
  $new_type = $form_state['values']['target_type'];
  $config = config('webformtonode.settings');
  $old_type = $config->get('mapping_' . $nid . '_type');
  $old_mapping = $config->get('mapping_' . $nid . '_fields');

  if (!empty($old_type) && $new_type != $old_type && !empty($old_mapping)) {
    $form_state['redirect'] = array("admin/config/webformtonode/settings/manage_webforms/fields/$nid/confirm-change", array('query' => array('new_type' => $new_type)));
    return;
  }

  $config->set('mapping_' . $nid . '_type', $new_type);
  $config->set('mapping_' . $nid . '_fields', $form_state['values']['mapping'] ?? array());
  $config->set('node_status_' . $nid, (int) ($form_state['values']['node_status'] ?? 1))->save();
  backdrop_set_message(t('Configuration saved.'));
  $form_state['redirect'] = 'admin/config/webformtonode/settings';
}

/**
 * Fallback confirmation form for type change.
 */
function webformtonode_admin_change_type_confirm_form($form, &$form_state, $nid) {
  $form['nid'] = array('#type' => 'value', '#value' => $nid);
  $form['new_type'] = array('#type' => 'value', '#value' => $_GET['new_type']);
  return confirm_form($form, t('Confirm Change?'), "admin/config/webformtonode/settings/manage_webforms/fields/$nid", t('Mappings will be cleared.'), t('Confirm'), t('Cancel'));
}

function webformtonode_admin_change_type_confirm_form_submit($form, &$form_state) {
  $nid = $form_state['values']['nid'];
  config('webformtonode.settings')->set('mapping_' . $nid . '_type', $form_state['values']['new_type'])->set('mapping_' . $nid . '_fields', array())->save();
  $form_state['redirect'] = "admin/config/webformtonode/settings/manage_webforms/fields/$nid";
}
