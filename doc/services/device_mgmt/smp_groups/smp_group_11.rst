.. _mcumgr_smp_group_11:

Transport Management Group
##########################

Transport management group defines the following commands:

.. table::
    :align: center

    +----------------+------------------------------------+
    | ``Command ID`` | Command description                |
    +================+====================================+
    | ``0``          | Connect (bridge) transport         |
    +----------------+------------------------------------+
    | ``1``          | Disconnect bridged transport       |
    +----------------+------------------------------------+
    | ``2``          | Fetch bridge/transport status      |
    +----------------+------------------------------------+
    | ``3``          | List transports                    |
    +----------------+------------------------------------+
    | ``4``          | Details on transports              |
    +----------------+------------------------------------+
    | ``5`` x         | Details on transport configuration |
    +----------------+------------------------------------+

Connect (bridge) transport command
**********************************

Bridge the transport which received the MCUmgr packet to another MCUmgr transport.

Connect (bridge) transport request
==================================

Connect (bridge) transport request header fields:

.. table::
    :align: center

    +--------+--------------+----------------+
    | ``OP`` | ``Group ID`` | ``Command ID`` |
    +========+==============+================+
    | ``2``  | ``11``       | ``0``          |
    +--------+--------------+----------------+

CBOR data of request:

.. code-block:: none

    {
        (str)"transport" : (uint)
        ...
    }

where:

.. table::
    :align: center

    +-------------+--------------------------------------------------------------+
    | "transport" | :c:enum:`smp_transport_type` contains the tranport type for  |
    |             | which to bridge (connect) from the transport to.             |
    +-------------+--------------------------------------------------------------+
    | ...         | there might be additional fields that the transport requires |
    |             | in order to make a connection, these are not described here  |
    |             | as are transport-specific.                                   |
    +-------------+--------------------------------------------------------------+

Connect (bridge) transport response
===================================

Connect (bridge) transport response header fields:

.. table::
    :align: center

    +--------+--------------+----------------+
    | ``OP`` | ``Group ID`` | ``Command ID`` |
    +========+==============+================+
    | ``3``  | ``11``       | ``0``          |
    +--------+--------------+----------------+

The command sends an empty CBOR map as data if successful.
In case of error the CBOR data takes the form:

.. tabs::

   .. group-tab:: SMP version 2

      .. code-block:: none

          {
              (str)"err" : {
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
    | "err" -> "group" | :c:enum:`mcumgr_group_t` group of the group-based error code. Only      |
    |                  | appears if an error is returned when using SMP version 2.               |
    +------------------+-------------------------------------------------------------------------+
    | "err" -> "rc"    | contains the index of the group-based error code. Only appears if       |
    |                  | non-zero (error condition) when using SMP version 2.                    |
    +------------------+-------------------------------------------------------------------------+
    | "rc"             | :c:enum:`mcumgr_err_t` only appears if non-zero (error condition) when  |
    |                  | using SMP version 1 or for SMP errors when using SMP version 2.         |
    +------------------+-------------------------------------------------------------------------+

Disconnect bridged transport command
************************************

Disconnect bridged transport request
====================================

Disconnect bridged transport request header fields:

Disconnect the current transport's bridge, or disconnect all transport bridges.

.. table::
    :align: center

    +--------+--------------+----------------+
    | ``OP`` | ``Group ID`` | ``Command ID`` |
    +========+==============+================+
    | ``2``  | ``11``       | ``1``          |
    +--------+--------------+----------------+

CBOR data of request:

.. tabs::

   .. group-tab:: Disconnect active bridge

      .. code-block:: none

          {
              (str)"transport" : (uint)
          }

   .. group-tab:: Disconnect all bridges

      .. code-block:: none

          {
              (str)"all" :       (bool)
          }

where:

.. table::
    :align: center

    +-------------+-------------------------------------------------------------+
    | "transport" | :c:enum:`smp_transport_type` contains the tranport type for |
    |             | which to bridge (connect) from the transport to, this must  |
    |             | not be provided if ``all`` is provided.                     |
    +-------------+-------------------------------------------------------------+
    | "all"       | set to true to disconnect all active bridged transports,    |
    |             | this must not be provided if ``transport`` is provided.     |
    +-------------+-------------------------------------------------------------+

Disconnect bridged transport response
=====================================

Disconnect bridged transport response header fields:

.. table::
    :align: center

    +--------+--------------+----------------+
    | ``OP`` | ``Group ID`` | ``Command ID`` |
    +========+==============+================+
    | ``3``  | ``11``       | ``1``          |
    +--------+--------------+----------------+

The command sends an empty CBOR map as data if successful.
In case of error the CBOR data takes the form:

.. tabs::

   .. group-tab:: SMP version 2

      .. code-block:: none

          {
              (str)"err" : {
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
    | "err" -> "group" | :c:enum:`mcumgr_group_t` group of the group-based error code. Only      |
    |                  | appears if an error is returned when using SMP version 2.               |
    +------------------+-------------------------------------------------------------------------+
    | "err" -> "rc"    | contains the index of the group-based error code. Only appears if       |
    |                  | non-zero (error condition) when using SMP version 2.                    |
    +------------------+-------------------------------------------------------------------------+
    | "rc"             | :c:enum:`mcumgr_err_t` only appears if non-zero (error condition) when  |
    |                  | using SMP version 1 or for SMP errors when using SMP version 2.         |
    +------------------+-------------------------------------------------------------------------+

Fetch bridged transport status command
**************************************

Return information on active bridges and what device supports.

Fetch bridged transport status request
======================================

Fetch bridged transport status request header fields:

.. table::
    :align: center

    +--------+--------------+----------------+
    | ``OP`` | ``Group ID`` | ``Command ID`` |
    +========+==============+================+
    | ``0``  | ``11``       | ``2``          |
    +--------+--------------+----------------+

The command sends an empty CBOR map as data.

Fetch bridged transport status response
=======================================

Fetch bridged transport status response header fields:

.. table::
    :align: center

    +--------+--------------+----------------+
    | ``OP`` | ``Group ID`` | ``Command ID`` |
    +========+==============+================+
    | ``1``  | ``11``       | ``2``          |
    +--------+--------------+----------------+

CBOR data of successful response:

.. code-block:: none

    {
        (str)"supported"      : (uint)
        (str)"active"         : (uint)
        (str,opt)"bridged"    : (bool)
        (str,opt)"transport"  : (uint)
    }

In case of error the CBOR data takes the form:

.. tabs::

   .. group-tab:: SMP version 2

      .. code-block:: none

          {
              (str)"err" : {
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

    +------------------+----------------------------------------------------------------------------+
    | "supported"      | contains how many bridges can be active at a given time.                   |
    +------------------+----------------------------------------------------------------------------+
    | "active"         | contains how many bridges are currently active.                            |
    +------------------+----------------------------------------------------------------------------+
    | "bridged"        | will be present and true if the current transport is bridged, otherwise    |
    |                  | will be omitted.                                                           |
    +------------------+----------------------------------------------------------------------------+
    | "transport"      | the transport ID of the MCUmgr transport that the transport is bridged to. |
    |                  | Only appears if the transport is bridged.                                  |
    +------------------+----------------------------------------------------------------------------+
    | "err" -> "group" | :c:enum:`mcumgr_group_t` group of the group-based error code. Only appears |
    |                  | if an error is returned when using SMP version 2.                          |
    +------------------+----------------------------------------------------------------------------+
    | "err" -> "rc"    | contains the index of the group-based error code. Only appears if non-zero |
    |                  | (error condition) when using SMP version 2.                                |
    +------------------+----------------------------------------------------------------------------+
    | "rc"             | :c:enum:`mcumgr_err_t` only appears if non-zero (error condition) when     |
    |                  | using SMP version 1 or for SMP errors when using SMP version 2.            |
    +------------------+----------------------------------------------------------------------------+

List transports command
***********************

Return information on transports that the device supports.

List transports request
=======================

List transports request header fields:

.. table::
    :align: center

    +--------+--------------+----------------+
    | ``OP`` | ``Group ID`` | ``Command ID`` |
    +========+==============+================+
    | ``0``  | ``11``       | ``3``          |
    +--------+--------------+----------------+

List transports response
========================

List transports response header fields:

.. table::
    :align: center

    +--------+--------------+----------------+
    | ``OP`` | ``Group ID`` | ``Command ID`` |
    +========+==============+================+
    | ``1``  | ``11``       | ``3``          |
    +--------+--------------+----------------+

CBOR data of successful response:

.. code-block:: none

    {
        (str)"transports" : [
            {
                (str)"id"         : (uint)
                (str,opt)"name"   : (str)
            }
            ...
        ]
    }

In case of error the CBOR data takes the form:

.. tabs::

   .. group-tab:: SMP version 2

      .. code-block:: none

          {
              (str)"err" : {
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
    | "id"             | the transport ID of the MCUmgr transport that supports bridging.        |
    +------------------+-------------------------------------------------------------------------+
    | "name"           | optional name of the MCUmgr transport (if available).                   |
    +------------------+-------------------------------------------------------------------------+
    | "err" -> "group" | :c:enum:`mcumgr_group_t` group of the group-based error code. Only      |
    |                  | appears if an error is returned when using SMP version 2.               |
    +------------------+-------------------------------------------------------------------------+
    | "err" -> "rc"    | contains the index of the group-based error code. Only appears if       |
    |                  | non-zero (error condition) when using SMP version 2.                    |
    +------------------+-------------------------------------------------------------------------+
    | "rc"             | :c:enum:`mcumgr_err_t` only appears if non-zero (error condition) when  |
    |                  | using SMP version 1 or for SMP errors when using SMP version 2.         |
    +------------------+-------------------------------------------------------------------------+

Details on transports command
*****************************

TODO

Return information on transports that the device supports.

Details on transports request
=============================

Details on transports request header fields:

.. table::
    :align: center

    +--------+--------------+----------------+
    | ``OP`` | ``Group ID`` | ``Command ID`` |
    +========+==============+================+
    | ``0``  | ``11``       | ``4``          |
    +--------+--------------+----------------+

Details on transports response
==============================

Details on transports response header fields:

.. table::
    :align: center

    +--------+--------------+----------------+
    | ``OP`` | ``Group ID`` | ``Command ID`` |
    +========+==============+================+
    | ``1``  | ``11``       | ``4``          |
    +--------+--------------+----------------+

CBOR data of successful response:

.. code-block:: none

    {
        (str)"modes" : [
            {
                (str)"type"          : (uint)
                (str)"description"   : (str)
                (str,opt)"incoming"  : (bool)
                (str,opt)"outgoing"  : (bool)
            }
            ...
        ]
        (str)"total"                 : (uint)
    }

In case of error the CBOR data takes the form:

.. tabs::

   .. group-tab:: SMP version 2

      .. code-block:: none

          {
              (str)"err" : {
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

    +------------------+-----------------------------------------------------------------------------+
    | "type"           | the type ID of the transport mode.                                          |
    +------------------+-----------------------------------------------------------------------------+
    | "description"    | description of the transport mode.                                          |
    +------------------+-----------------------------------------------------------------------------+
    | "incoming"       | will be set to true if transport mode supports incoming bridge connections. |
    +------------------+-----------------------------------------------------------------------------+
    | "outgoing"       | will be set to true if transport mode supports outgoing bridge connections. |
    +------------------+-----------------------------------------------------------------------------+
    | "total"          | total number of supported modes for this transport.                         |
    +------------------+-----------------------------------------------------------------------------+
    | "err" -> "group" | :c:enum:`mcumgr_group_t` group of the group-based error code. Only appears  |
    |                  | if an error is returned when using SMP version 2.                           |
    +------------------+-----------------------------------------------------------------------------+
    | "err" -> "rc"    | contains the index of the group-based error code. Only appears if non-zero  |
    |                  | (error condition) when using SMP version 2.                                 |
    +------------------+-----------------------------------------------------------------------------+
    | "rc"             | :c:enum:`mcumgr_err_t` only appears if non-zero (error condition) when      |
    |                  | using SMP version 1 or for SMP errors when using SMP version 2.             |
    +------------------+-----------------------------------------------------------------------------+


Details on transport configuration command
******************************************

TODO

Return information on transport configuration that is supported.

Details on transport configuration request
==========================================

Details on transport configuration request header fields:

.. table::
    :align: center

    +--------+--------------+----------------+
    | ``OP`` | ``Group ID`` | ``Command ID`` |
    +========+==============+================+
    | ``0``  | ``11``       | ``5``          |
    +--------+--------------+----------------+

Details on transport configuration response
===========================================

Details on transport configuration response header fields:

.. table::
    :align: center

    +--------+--------------+----------------+
    | ``OP`` | ``Group ID`` | ``Command ID`` |
    +========+==============+================+
    | ``1``  | ``11``       | ``5``          |
    +--------+--------------+----------------+

TODO

CBOR data of successful response:

.. code-block:: none

    {
        (str)"modes" : [
            {
                (str)"type"          : (uint)
                (str)"description"   : (str)
            }
            ...
        ]
        (str)"total"                 : (uint)
    }

In case of error the CBOR data takes the form:

.. tabs::

   .. group-tab:: SMP version 2

      .. code-block:: none

          {
              (str)"err" : {
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

TODO

    +------------------+-------------------------------------------------------------------------+
    | "type"           | the type ID of the transport mode.                                      |
    +------------------+-------------------------------------------------------------------------+
    | "description"    | description of the transport mode.                                      |
    +------------------+-------------------------------------------------------------------------+
    | "total"          | total number of supported modes for this transport.                     |
    +------------------+-------------------------------------------------------------------------+
    | "err" -> "group" | :c:enum:`mcumgr_group_t` group of the group-based error code. Only      |
    |                  | appears if an error is returned when using SMP version 2.               |
    +------------------+-------------------------------------------------------------------------+
    | "err" -> "rc"    | contains the index of the group-based error code. Only appears if       |
    |                  | non-zero (error condition) when using SMP version 2.                    |
    +------------------+-------------------------------------------------------------------------+
    | "rc"             | :c:enum:`mcumgr_err_t` only appears if non-zero (error condition) when  |
    |                  | using SMP version 1 or for SMP errors when using SMP version 2.         |
    +------------------+-------------------------------------------------------------------------+
