<?php

/**
 * @file
 *   Provides a admin section to allow editing social settings
 */

/**
 * Generates the social setting type editing form.
 */
function social_settings_type_form($form, &$form_state, $task_type, $op = 'edit') {

  if ($op == 'clone') {
    $task_type->label .= ' (cloned)';
    $task_type->type = '';
  }

  $form['label'] = array(
    '#title' => t('Label'),
    '#type' => 'textfield',
    '#default_value' => $task_type->label,
    '#description' => t('The human-readable name of this social setting type.'),
    '#required' => TRUE,
    '#size' => 30,
  );

  // Machine-readable type name.
  $form['type'] = array(
    '#type' => 'machine_name',
    '#default_value' => isset($task_type->type) ? $task_type->type : '',
    '#maxlength' => 32,
    '#disabled' => $task_type->isLocked() && $op != 'clone',
    '#machine_name' => array(
      'exists' => 'social_settings_types',
      'source' => array('label'),
    ),
    '#description' => t('A unique machine-readable name for this social setting type. It must only contain lowercase letters, numbers, and underscores.'),
  );

  $form['description'] = array(
    '#type' => 'textarea',
    '#default_value' => isset($task_type->description) ? $task_type->description : '',
    '#description' => t('Description about the social setting type.'),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save social setting type'),
    '#weight' => 40,
  );

  if (!$task_type->isLocked() && $op != 'add' && $op != 'clone') {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete social setting type'),
      '#weight' => 45,
      '#limit_validation_errors' => array(),
      '#submit' => array('social_settings_type_form_submit_delete')
    );
  }
  return $form;
}

/**
 * Submit handler for creating/editing social_settings_type.
 */
function social_settings_type_form_submit(&$form, &$form_state) {
  $task_type = entity_ui_form_submit_build_entity($form, $form_state);
  // Save and go back.
  social_settings_type_save($task_type);

  // Redirect user back to list of task types.
  $form_state['redirect'] = 'admin/social-settings/type';
}

function social_settings_type_form_submit_delete(&$form, &$form_state) {
  $form_state['redirect'] = 'admin/social-settings/type' . $form_state['social_settings_type']->type . '/delete';
}

/**
 * Social settings type delete form.
 */
function social_settings_type_form_delete_confirm($form, &$form_state, $task_type) {
  $form_state['social_settings_type'] = $task_type;
  // Always provide entity id in the same form key as in the entity edit form.
  $form['social_settings_type_id'] = array('#type' => 'value', '#value' => entity_id('social_settings_type', $task_type));
  return confirm_form($form,
    t('Are you sure you want to delete task type %title?', array('%title' => entity_label('social_settings_type', $task_type))),
    'admin/social-settings/' . entity_id('social_settings_type', $task_type),
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Social settings type delete form submit handler.
 */
function social_settings_type_form_delete_confirm_submit($form, &$form_state) {
  $task_type = $form_state['social_settings_type'];
  social_settings_type_delete($task_type);

  watchdog('social_settings_type', '@type: deleted %title.', array('@type' => $task_type->type, '%title' => $task_type->label));
  drupal_set_message(t('@type %title has been deleted.', array('@type' => $task_type->type, '%title' => $task_type->label)));

  $form_state['redirect'] = 'admin/social-settings/type';
}

/**
 * Page to select social settings type to add new social setting.
 */
function social_settings_admin_add_page() {
  $items = array();
  foreach (social_settings_types() as $social_settings_type_key => $social_settings_type) {
    $items[] = l(entity_label('social_settings_type', $social_settings_type), 'admin/social-settings/add/' . $social_settings_type_key);
  }
  return array('list' => array('#theme' => 'item_list', '#items' => $items, '#title' => t('Select type of social setting to create.')));
}

/**
 * Add new social setting page callback.
 */
function social_settings_add($type) {
  $task_type = social_settings_types($type);

  $task = entity_create('social_settings', array('type' => $type));
  drupal_set_title(t('Create @name', array('@name' => entity_label('task_type', $task_type))));

  $output = drupal_get_form('social_settings_form', $task);

  return $output;
}

/**
 * Social settings form.
 */
function social_settings_form($form, &$form_state, $task) {
  $form_state['social_settings'] = $task;

  $form['title'] = array(
    '#type' => 'textfield',
    '#required' => TRUE,
    '#title' => t('Name'),
    '#default_value' => $task->title,
  );

  $form['description'] = array(
    '#type' => 'textarea',
    '#title' => t('Description'),
    '#default_value' => $task->description,
  );

  $form['uid'] = array(
    '#type' => 'value',
    '#value' => $task->uid,
  );

  field_attach_form('social_settings', $task, $form, $form_state);

  $submit = array();
  if (!empty($form['#submit'])) {
    $submit += $form['#submit'];
  }

  $form['actions'] = array(
    '#weight' => 100,
  );

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save task'),
    '#submit' => $submit + array('social_settings_form_submit'),
  );

  // Show Delete button if we edit task.
  $task_id = entity_id('social_settings', $task);
  if (!empty($task_id) && social_settings_access('edit', $task)) {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#submit' => array('social_settings_form_submit_delete'),
    );
  }

  $form['#validate'][] = 'social_settings_form_validate';

  return $form;
}

function social_settings_form_validate($form, &$form_state) {

}

/**
 * Social settings submit handler.
 */
function social_settings_form_submit($form, &$form_state) {
  $task = $form_state['social_settings'];

  entity_form_submit_build_entity('social_settings', $task, $form, $form_state);

  social_settings_save($task);

  $task_uri = entity_uri('social_settings', $task);

  $form_state['redirect'] = $task_uri['path'];

  drupal_set_message(t('Social settings %title saved.', array('%title' => entity_label('social_settings', $task))));
}

function social_settings_form_submit_delete($form, &$form_state) {
  $task = $form_state['social_settings'];
  $task_uri = entity_uri('social_settings', $task);
  $form_state['redirect'] = $task_uri['path'] . '/delete';
}

/**
 * Delete confirmation form.
 */
function social_settings_delete_form($form, &$form_state, $task) {
  $form_state['social_settings'] = $task;
  // Always provide entity id in the same form key as in the entity edit form.
  $form['social_settings_type_id'] = array('#type' => 'value', '#value' => entity_id('social_settings', $task));
  $task_uri = entity_uri('social_settings', $task);
  return confirm_form($form,
    t('Are you sure you want to delete social setting %title?', array('%title' => entity_label('social_settings', $task))),
    $task_uri['path'],
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Delete form submit handler.
 */
function social_settings_delete_form_submit($form, &$form_state) {
  $task = $form_state['social_settings'];
  social_settings_delete($task);

  drupal_set_message(t('Social setting %title deleted.', array('%title' => entity_label('social_settings', $task))));

  $form_state['redirect'] = 'admin/social-settings/';
}
