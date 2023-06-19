.. _mcumgr_smp_group_3:

Settings (Config) Management Group
##################################

Settings management group (known as Configuration Manager in the original MCUmgr repository)
defines the following commands:

.. table::
    :align: center

    +----------------+------------------------------+
    | ``Command ID`` | Command description          |
    +================+==============================+
    | ``0``          | Read/write setting           |
    +----------------+------------------------------+
    | ``1``          | Delete setting               |
    +----------------+------------------------------+
    | ``2``          | Commit settings              |
    +----------------+------------------------------+
    | ``3``          | Save settings                |
    +----------------+------------------------------+

Note that the Zephyr version adds additional commands and features which are not supported by
the original upstream version, however, the original client functionality should work for
read/write functionality.

Read/write setting command
**************************

Read/write setting command allows updating a setting entry on a device or
getting the current value of a setting from a device.

Read setting request
====================

Read setting request header fields:

.. table::
    :align: center

    +--------+--------------+----------------+
    | ``OP`` | ``Group ID`` | ``Command ID`` |
    +========+==============+================+
    | ``0``  | ``3``        | ``0``          |
    +--------+--------------+----------------+

CBOR data of request:

.. code-block:: none

    {
        (str)"name"         : (str)
        (str,opt)"max_size" : (uint)
    }

where:

.. table::
    :align: center

    +------------+-----------------------------------------+
    | "name"     | string of the setting to retrieve       |
    +------------+-----------------------------------------+
    | "max_size" | optional maximum size of data to return |
    +------------+-----------------------------------------+

Read setting response
=====================

Read setting response header fields:

.. table::
    :align: center

    +--------+--------------+----------------+
    | ``OP`` | ``Group ID`` | ``Command ID`` |
    +========+==============+================+
    | ``1``  | ``3``        | ``0``          |
    +--------+--------------+----------------+

CBOR data of successful response:

.. code-block:: none

    {
        (str)"val"         : (bstr)
        (str,opt)"limited" : (bool)
    }

In case of error the CBOR data takes the form:

.. tabs::

   .. group-tab:: SMP version 2

      .. code-block:: none

          {
              (str)"ret" : {
                  (str)"group"    : (uint)
                  (str)"rc"       : (uint)
              }
          }

   .. group-tab:: SMP version 1

      .. code-block:: none

          {
              (str)"rc"       : (int)
          }

where:

.. table::
    :align: center

    +------------------+-------------------------------------------------------------------------+
    | "val"            | binary string of the returned data, note that the underlying data type  |
    |                  | cannot be specified through this and must be known by the client.       |
    +------------------+-------------------------------------------------------------------------+
    | "limited"        | will be set to true if the settings value is larger than the maximum    |
    |                  | requested (or supported via MCUmgr setting                              |
    |                  | :kconfig:option:`CONFIG_MCUMGR_GRP_SETTINGS_NAME_LEN`) data length.     |
    +------------------+-------------------------------------------------------------------------+
    | "ret" -> "group" | :c:enum:`mcumgr_group_t` group of the group-based error code. Only      |
    |                  | appears if an error is returned when using SMP version 2.               |
    +------------------+-------------------------------------------------------------------------+
    | "ret" -> "rc"    | contains the index of the group-based error code. Only appears if       |
    |                  | non-zero (error condition) when using SMP version 2.                    |
    +------------------+-------------------------------------------------------------------------+
    | "rc"             | :c:enum:`mcumgr_err_t` only appears if non-zero (error condition) when  |
    |                  | using SMP version 1.                                                    |
    +------------------+-------------------------------------------------------------------------+

Write setting request
=====================

Write setting request header fields:

.. table::
    :align: center

    +--------+--------------+----------------+
    | ``OP`` | ``Group ID`` | ``Command ID`` |
    +========+==============+================+
    | ``2``  | ``3``        | ``0``          |
    +--------+--------------+----------------+

CBOR data of request:

.. code-block:: none

    {
        (str)"name"    : (str)
        (str)"val"     : (bstr)
    }

where:

.. table::
    :align: center

    +--------+-------------------------------------+
    | "name" | string of the setting to update/set |
    +--------+-------------------------------------+
    | "val"  | value to set the setting to         |
    +--------+-------------------------------------+

Write setting response
======================

Write setting response header fields:

.. table::
    :align: center

    +--------+--------------+----------------+
    | ``OP`` | ``Group ID`` | ``Command ID`` |
    +========+==============+================+
    | ``3``  | ``3``        | ``0``          |
    +--------+--------------+----------------+

The command sends an empty CBOR map as data if successful. In case of error the CBOR data takes
the form:

.. tabs::

   .. group-tab:: SMP version 2

      .. code-block:: none

          {
              (str)"ret" : {
                  (str)"group"    : (uint)
                  (str)"rc"       : (uint)
              }
          }

   .. group-tab:: SMP version 1

      .. code-block:: none

          {
              (str)"rc"       : (int)
          }

where:

.. table::
    :align: center

    +------------------+-------------------------------------------------------------------------+
    | "ret" -> "group" | :c:enum:`mcumgr_group_t` group of the group-based error code. Only      |
    |                  | appears if an error is returned when using SMP version 2.               |
    +------------------+-------------------------------------------------------------------------+
    | "ret" -> "rc"    | contains the index of the group-based error code. Only appears if       |
    |                  | non-zero (error condition) when using SMP version 2.                    |
    +------------------+-------------------------------------------------------------------------+
    | "rc"             | :c:enum:`mcumgr_err_t` only appears if non-zero (error condition) when  |
    |                  | using SMP version 1.                                                    |
    +------------------+-------------------------------------------------------------------------+

Delete setting command
**********************

Delete setting command allows deleting a setting on a device.

Delete setting request
======================

Delete setting request header fields:

.. table::
    :align: center

    +--------+--------------+----------------+
    | ``OP`` | ``Group ID`` | ``Command ID`` |
    +========+==============+================+
    | ``2``  | ``3``        | ``1``          |
    +--------+--------------+----------------+

CBOR data of request:

.. code-block:: none

    {
        (str)"name"   : (str)
    }

where:

.. table::
    :align: center

    +--------+---------------------------------+
    | "name" | string of the setting to delete |
    +--------+---------------------------------+

Delete setting response
=======================

Delete setting response header fields:

.. table::
    :align: center

    +--------+--------------+----------------+
    | ``OP`` | ``Group ID`` | ``Command ID`` |
    +========+==============+================+
    | ``3``  | ``3``        | ``1``          |
    +--------+--------------+----------------+

The command sends an empty CBOR map as data if successful. In case of error the CBOR data takes
the form:

.. tabs::

   .. group-tab:: SMP version 2

      .. code-block:: none

          {
              (str)"ret" : {
                  (str)"group"    : (uint)
                  (str)"rc"       : (uint)
              }
          }

   .. group-tab:: SMP version 1

      .. code-block:: none

          {
              (str)"rc"       : (int)
          }

where:

.. table::
    :align: center

    +------------------+-------------------------------------------------------------------------+
    | "ret" -> "group" | :c:enum:`mcumgr_group_t` group of the group-based error code. Only      |
    |                  | appears if an error is returned when using SMP version 2.               |
    +------------------+-------------------------------------------------------------------------+
    | "ret" -> "rc"    | contains the index of the group-based error code. Only appears if       |
    |                  | non-zero (error condition) when using SMP version 2.                    |
    +------------------+-------------------------------------------------------------------------+
    | "rc"             | :c:enum:`mcumgr_err_t` only appears if non-zero (error condition) when  |
    |                  | using SMP version 1.                                                    |
    +------------------+-------------------------------------------------------------------------+

Commit settings command
***********************

Commit settings command allows committing all settings that have been set but not yet applied on a
device.

Commit settings request
=======================

Commit settings request header fields:

.. table::
    :align: center

    +--------+--------------+----------------+
    | ``OP`` | ``Group ID`` | ``Command ID`` |
    +========+==============+================+
    | ``2``  | ``3``        | ``2``          |
    +--------+--------------+----------------+

The command sends sends empty CBOR map as data.

Commit settings response
========================

Commit settings response header fields:

.. table::
    :align: center

    +--------+--------------+----------------+
    | ``OP`` | ``Group ID`` | ``Command ID`` |
    +========+==============+================+
    | ``3``  | ``3``        | ``2``          |
    +--------+--------------+----------------+

The command sends an empty CBOR map as data if successful. In case of error the CBOR data takes
the form:

.. tabs::

   .. group-tab:: SMP version 2

      .. code-block:: none

          {
              (str)"ret" : {
                  (str)"group"    : (uint)
                  (str)"rc"       : (uint)
              }
          }

   .. group-tab:: SMP version 1

      .. code-block:: none

          {
              (str)"rc"       : (int)
          }

where:

.. table::
    :align: center

    +------------------+-------------------------------------------------------------------------+
    | "ret" -> "group" | :c:enum:`mcumgr_group_t` group of the group-based error code. Only      |
    |                  | appears if an error is returned when using SMP version 2.               |
    +------------------+-------------------------------------------------------------------------+
    | "ret" -> "rc"    | contains the index of the group-based error code. Only appears if       |
    |                  | non-zero (error condition) when using SMP version 2.                    |
    +------------------+-------------------------------------------------------------------------+
    | "rc"             | :c:enum:`mcumgr_err_t` only appears if non-zero (error condition) when  |
    |                  | using SMP version 1.                                                    |
    +------------------+-------------------------------------------------------------------------+

Save settings command
*********************

Save settings command allows storing all serialized items to persistent storage on a device.

Save settings request
=====================

Save settings request header fields:

.. table::
    :align: center

    +--------+--------------+----------------+
    | ``OP`` | ``Group ID`` | ``Command ID`` |
    +========+==============+================+
    | ``2``  | ``3``        | ``3``          |
    +--------+--------------+----------------+

The command sends sends empty CBOR map as data.

Save settings response
======================

Save settings response header fields:

.. table::
    :align: center

    +--------+--------------+----------------+
    | ``OP`` | ``Group ID`` | ``Command ID`` |
    +========+==============+================+
    | ``3``  | ``3``        | ``3``          |
    +--------+--------------+----------------+

The command sends an empty CBOR map as data if successful. In case of error the CBOR data takes
the form:

.. tabs::

   .. group-tab:: SMP version 2

      .. code-block:: none

          {
              (str)"ret" : {
                  (str)"group"    : (uint)
                  (str)"rc"       : (uint)
              }
          }

   .. group-tab:: SMP version 1

      .. code-block:: none

          {
              (str)"rc"       : (int)
          }

where:

.. table::
    :align: center

    +------------------+-------------------------------------------------------------------------+
    | "ret" -> "group" | :c:enum:`mcumgr_group_t` group of the group-based error code. Only      |
    |                  | appears if an error is returned when using SMP version 2.               |
    +------------------+-------------------------------------------------------------------------+
    | "ret" -> "rc"    | contains the index of the group-based error code. Only appears if       |
    |                  | non-zero (error condition) when using SMP version 2.                    |
    +------------------+-------------------------------------------------------------------------+
    | "rc"             | :c:enum:`mcumgr_err_t` only appears if non-zero (error condition) when  |
    |                  | using SMP version 1.                                                    |
    +------------------+-------------------------------------------------------------------------+
