<?php

/**
 * @file
 *   Provides a admin section to allow editing social settings
 * 
 * @author
 *   James Milward
 */

/**
 * Task view callback.
 */
function social_settings_view($task) {
  drupal_set_title(entity_label('social_settings', $task));
  return entity_view('social_settings', array(entity_id('social_settings', $task) => $task), 'full');
}
