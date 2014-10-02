
This module utilises the Entity API in Drupal to provide a fully customisable 
interface to add / remove necessary fields for integrating Drupal with social
networks.

This module will provide all functionality through /admin/social_settings.
Security can be configured through the roles/permission interface in Drupal, 
this is all defined in social_settings_permission() within the .module file.

Accessing this module in Drupal
-------------------------------

This is simply achieved by calling -

$social_settings = entity_load('social_settings');

... and then traversing through this.


