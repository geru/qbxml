CONTENTS OF THIS FILE
---------------------

 * Introduction
 * Requirements
 * Recommended modules
 * Installation
 * Configuration
 * Troubleshooting
 * FAQ
 * Maintainers

INTRODUCTION
------------

The Quickbooks XML (QBXML) module performs input and output operations relating to the Drupal site and the remote
Quickbooks site.

On the input side, QBXML uses the Feeds module to convert incoming QBXML messages from QBXML format into formats and
data structures known to the Drupal site.

On the output side, QBXML implements the QBXML theme hook to convert Drupal arrays into Quickbooks XML. QBXML also makes
various theme hook suggestions and scans for implemented QBXML template files to implement more hooks.

REQUIREMENTS
------------

QBXML requires the Quickbooks Webconnector (QBWC) module. QBXML uses hooks to and from the Quickbooks Webconnector
(QBWC) to communicate with Quickbooks. So, no communications with Quickbooks will happen without successfully installing
and configuring the QBWC module.

RECOMMENDED MODULES
-------------------

QBXML is designed with the Feeds module in mind. It also defines a default import function that uses Feeds. Without the
Feeds module installed, you will need to implement your own import functionality.

Finally, there needs to be some place for the data to go when it comes in. The bc module provides default Drupal data
structures to store much of the Quickbooks data as well as a bc_import module that defines Feeds importers for these
data structures.

QBWC, QBXML, and BC all use a BookID model for collating input and output and associating the data streams with
the proper data sets in Drupal.


INSTALLATION
------------

QBXML is installed just like any normal Drupal module.

CONFIGURATION
-------------

If you will be using the standard Feeds-based importer defined by QBXML, then you may wish to configure the directory
used to store incoming and outgoing QBXML messages. These files contain private, sensitive data and so the directory
should be secured from prying eyes. By default, QBXML uses a directory called "qbxml" in the private: uri. So, if you
have not configured your private directory (this is the case in Drupal by default), then you will get many error
messages until you define the private directory in /admin/config/media/file-system.

QBXML also defines the QBXML theme for formatting outgoing QBXML messages.

You may extend this module by writing your own qbxml.tpl.php theme files. If you create a file called
mytheme.qbxml.tpl.php and put it in either the sites/all/libraries/qbxml or sites/all/themes/qbxml directories, then the
QBXML module will find the file and automatically create a theme hook called 'qbxml__mytheme' using that theme file.
This allows QBXML to be extended simply by writing new, appropriate theme files and no need to add or enable modules,
themes, or libraries in Drupal.

You can also extend the QBXML theme system by writing a custom module with formally implemented theme hooks. Of course,
you will need to clear your theme cache after adding a new theme file.

The standard QBXML import function uses appropriately named feeds importers to process incoming Quickbooks XML messages.
Quickbooks XML messages are in XML format with a top-level tag such as <AccountQueryRs> in response to an
<AccountQueryRq> request.

The QBXML import function examines that top-level tag (in the example above, this would be 'AccountQueryRs'. It looks
for a feeds importer called qbxml_import_accountqueryrs. If it finds this importer, it will import the message using
that importer. If it doesn't find that importer, it will look for one called qbxml_import_accountquery, and failing
that, it will look for one called qbxml_import_account. Since most of the Quickbooks requests are of the form
ListnameQueryRq, or ListnameAddRq, or ListnameModRq and each of these uses a response list of <ListnameRet> elements,
you would probably implement importers called qbxml_import_Listname and then make a custom importer for a Query, Mod, or
Add response only as needed.

The prefix 'qbxml_import' is the default prefix defined by and used by the QBXML module. However, this prefix can be
overridden when a request is initiated.

Because a Feeds importer must know the internal data representations, and the QBXML makes no assumptions about the
internal data representation, QBXML doesn't define any importers. You therefore need some other module to create the
internal data structure and provide the importer for the data.

The Bc module provides data structures and importers for many Quickbooks datasets.

Therefore, if you install this module, the QBWC module, and the Bc and BcFeeds modules, you would have a fully working
system for synchronizing data between Drupal and Quickbooks. You are also welcome to implement your own modules for
storing and importing data.
