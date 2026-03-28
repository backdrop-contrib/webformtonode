# Webform submission to Node
## This is an early Alpha prototype... Do not use - Do not install - This is for collaboration between developers only.

A module to copy a Webform submission to a new Node.  Field mapping allow you to map a Webform field with the same field type as the node.



## Requirements:
Webform

## Installation:
Install this module using the official Backdrop CMS instructions at https://docs.backdropcms.org/documentation/extend-with-modules

## Configuration - Setup - Mapping

Visit the configuration page under Administration > Webform to Node (/admin/config/webformtonode/settings).

- Use the "Manage webform selection" link to select a Webform.
- Then you can "configure" the field mappings.
- Select the "Target Content Type" and Webform components will be displayed with selection boxes to select which field you woudl like mapping to that Webform component.
- The "Additional Processing Options" allow you to select if the new node should be "Published" or "unpublished".

## Save to node
- Visit the Webform submissions page.
- View a submission.
- Click "Save to Node"
- Choose the appropriate option (Save - Save and Edit - Cancel)


## Webform fields types supported
 - Textfield
 - Textarea
 - Email
 - Links
 - Images

## Documentation: [todo]
Additional documentation is located in the Wiki: https://github.com/backdrop-contrib/webformtonode/wiki

## Issues:
Bugs and Feature requests should be reported in the Issue Queue: https://github.com/backdrop-contrib/webformtonode/issues

## Current Maintainer(s):
- [Steve Moorhouse (albanycomputers)] (https://github.com/albanycomputers)
- Seeking additional maintainers / contributors.

## Credits:
- Backdrop CMS Module created by Stephen Moorhouse.

- Thank you to the following people on Zulip for assistance while creating this module, I couldn't have done it without you.
- * argiepiano
- * indigoxela
- * klonos
- * findlabnet (Vladimir)
- * BWPanda

## Sponsorship:
 - [Albany Computer Services] (https://www.albany-computers.co.uk)
 - [Albany Hosting] (https://www.albany-hosting.co.uk)

License
This project is GPL2 software. See the LICENSE.txt file in this directory for complete text.
