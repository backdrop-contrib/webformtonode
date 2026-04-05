# Webform to Node

## Description
Webform to Node provides a seamless "Promotion" workflow for Backdrop CMS. It allows site administrators to map Webform submissions to specific Content Types and promote individual submissions into full-fledged Nodes at the click of a button.

What sets this module apart is its **Pluggable Architecture**. Built with developers in mind, it features a discovery engine that allows other modules to register custom field handlers, making it infinitely expandable for complex data types or external entity mapping.

**Status: Beta Release (v0.2.0)**
> **WARNING:** This is a Beta release. While the core engine is stable, please test thoroughly on a staging environment before deploying to a high-traffic production site. Always back up your database before installation.

---

## Features
* **Unified Command Center:** Manage all your webform-to-node mappings from a single administrative dashboard.
* **Intelligent Field Mapping:** Match Webform components to Node fields based on compatible data types.
* **Developer Friendly:** Easily extend the module using `hook_webformtonode_handler_info()`.
* **Data Integrity:** Automated "Janitor" hooks ensure that if a node or submission is deleted, the mapping registry is cleaned up instantly.
* **AJAX-Powered Workflow:** Promote submissions without leaving the results page via a sleek modal interface.

---

## Requirements
* [Webform](https://backdropcms.org/project/webform)
* [Entity Plus](https://backdropcms.org/project/entity_plus)

---

## Installation
1.  Install this module using the official Backdrop CMS instructions at [https://docs.backdropcms.org/documentation/extend-with-modules](https://docs.backdropcms.org/documentation/extend-with-modules).
2.  Navigate to **Administer > People > Permissions** and grant the **"Promote webform submissions to nodes"** permission to the appropriate roles.

---

## Configuration & Setup

### 1. The Command Center
Visit **Administration > Configuration > Webform to Node** (`admin/config/webformtonode/settings`).

* **Manage Webform Selection:** Select which Webforms should be enabled for the promotion engine.
* **Field Mapping:** Click "Configure" next to an enabled webform. Select your **Target Content Type** to reveal the mapping table.
* **Additional Processing:** Set default behaviours, such as whether promoted nodes should be "Published" or "Unpublished" by default.

### 2. Handler Management
The **Field Handler Management** section displays all discovered "Expert" handlers. If you add a new handler file to the `/handlers` folder or install a module that provides a new handler hook, use the **Rebuild Handler Registry** button to register it.

---

## Usage: Promoting a Submission
1.  Visit the **Webform Results** or a specific **Submission** page.
2.  Click the **Save as node** button at the top of the submission.
3.  In the modal window, choose:
    * **Save:** Creates the node and returns you to the results.
    * **Save and Edit:** Creates the node and takes you directly to the node edit form.
4.  Once promoted, the button will toggle to **View created node**.

---

## Developer Architecture (Pluggable Handlers)
External modules (e.g., your `abc_` or `xyz_` modules) can register custom mapping logic using the provided hook:

```php
/**
 * Implements hook_webformtonode_handler_info().
 */
function mymodule_webformtonode_handler_info() {
  return array(
    'my_custom_field' => array(
      'label' => t('My Expert Handler'),
      'file' => 'handlers/my_custom_field', // Path to your .inc file
      'module' => 'mymodule',
    ),
  );
}
```

## Roadmap / Todo
[ ] Account Creation: Implement the ability to create/map User Accounts directly from submissions.

[ ] Bulk Promotion: Integration with Views Bulk Operations (VBO) to promote multiple submissions at once.

[ ] Conditional Mapping: Allow mappings based on webform component values.

[ ] Database Standards: Audit and update all registry/mapping code to strictly utilise Backdrop's database abstraction layer and schema API.

## Documentation
Additional documentation and developer guides are located in the Wiki: https://github.com/backdrop-contrib/webformtonode/wiki

## Issues
Bugs and feature requests should be reported in the Issue Queue: https://github.com/backdrop-contrib/webformtonode/issues

## Current Maintainer(s):
- Steve Moorhouse (albanycomputers) (https://github.com/albanycomputers)
- Additional maintainers and contributors are welcome.

## Credits
- Steve Moorhouse - Zulip (DrAlbany)
- Google Gemini 3.0 assisted with the coding of this module.

**Special thanks to the Backdrop Zulip community for architectural guidance: argiepiano, indigoxela, klonos, findlabnet, and BWPanda.**

## Sponsorship:
- Albany Computer Services (https://www.albany-computers.co.uk)
- Albany Web Design (https://www.albanywebdesign.co.uk)
- Albany Hosting (https://www.albany-hosting.co.uk)

## License
This project is GPL v2 software. See the LICENSE.txt file in this directory for complete text.
