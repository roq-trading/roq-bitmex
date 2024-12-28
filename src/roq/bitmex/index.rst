.. _roq-bitmex:

.. |checkmark| unicode:: U+2713

roq-bitmex
==========


Links
-----

* `Website <https://www.bitmex.com/>`__
* `Testnet <https://testnet.bitmex.com/>`__
* `Support <https://www.bitmex.com/app/support>`__
* `Documentation <https://www.bitmex.com/app/apiOverview>`__


Supports
--------

.. grid::  2
  :gutter: 2

  .. grid-item-card::  Products

    .. list-table::
      :widths: auto

      * - Spot
        - |checkmark|
      * - Futures
        - |checkmark|
      * - Options
        -
      * - Combos
        -

  .. grid-item-card::  Market Data

    .. list-table::
      :widths: auto

      * - Reference Data
        - |checkmark|
      * - Market Status
        - |checkmark|
      * - Top of Book
        - |checkmark|
      * - Market by Price
        - |checkmark|
      * - Market by Order
        -
      * - Trade Summary
        - |checkmark|
      * - Statistics
        - |checkmark|

  .. grid-item-card::  Order Management

    .. list-table::
      :widths: auto

      * - Create
        - |checkmark|
      * - Modify
        - |checkmark|
      * - Cancel
        - |checkmark|
      * - Cancel All
        - |checkmark|
      * - Auto-Cancel
        -

  .. grid-item-card::  Account Management

    .. list-table::
      :widths: auto

      * - Positions
        - |checkmark|
      * - Funds
        -


Installing
----------

* :ref:`Using Conda <tutorial-conda>`

.. tab:: Stable

  .. code-block:: shell

     $ conda install \
           --channel https://roq-trading.com/conda/stable \
           roq-bitmex

.. tab:: Unstable

  .. code-block:: shell

     $ conda install \
           --channel https://roq-trading.com/conda/unstable \
           roq-bitmex


Using
-----

.. code-block:: shell

   $ roq-bitmex \
         --name "bitmex" \
         --config_file $CONFIG_FILE_PATH \
         --client_listen_address $UNIX_SOCKET_PATH \
         --flagfile $ENVIRONMENT_FLAGFILE


.. _roq-bitmex-flags:

Flags
-----

* :ref:`Using Flags <abseil-cpp>`
* :ref:`Gateway Flags <gateway-flags>`

.. code-block:: shell

   $ roq-bitmex --help

.. tab:: Flags

   .. include:: flags/flags.rstinc

.. tab:: REST

   .. include:: flags/rest.rstinc

.. tab:: WS

   .. include:: flags/ws.rstinc

.. tab:: Misc

   .. include:: flags/misc.rstinc


Environments
------------

.. tab:: Prod

   .. code-block:: shell

      $ $CONDA_PREFIX/share/roq-bitmex/flags/prod/flags.cfg

   .. include:: flags/prod/flags.cfg
     :code: shell

.. tab:: Test

   .. code-block:: shell

      $ $CONDA_PREFIX/share/roq-bitmex/flags/test/flags.cfg

   .. include:: flags/test/flags.cfg
     :code: shell


Configuration
-------------

* :ref:`Gateway Config <gateway-config>`

.. code-block:: shell

   $ $CONDA_PREFIX/share/roq-bitmex/config.toml

.. important::

   The template will be replaced when the software is upgraded.
   Make a copy and modify to your needs.

.. include:: config.toml
   :code: toml


Market Data
-----------

.. tab:: Live

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::ReferenceData`
      - MarketData
      - instrument
      -

    * - :cpp:class:`roq::MarketStatus`
      - MarketData
      - instrument
      -

    * - :cpp:class:`roq::TopOfBook`
      - MarketData
      - quote
      -

    * - :cpp:class:`roq::MarketByPriceUpdate`
      - MarketData
      - orderBookL2
      -

    * - :cpp:class:`roq::MarketByOrderUpdate`
      -
      -
      - Unavailable

    * - :cpp:class:`roq::TradeSummary`
      - MarketData
      - trade
      -

    * - :cpp:class:`roq::StatisticsUpdate`
      - MarketData
      - instrument
      -

.. tab:: Download

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::ReferenceData`
      -
      -
      -

    * - :cpp:class:`roq::MarketStatus`
      -
      -
      -

    * - :cpp:class:`roq::TopOfBook`
      -
      -
      -

    * - :cpp:class:`roq::MarketByPriceUpdate`
      -
      -
      -

    * - :cpp:class:`roq::MarketByOrderUpdate`
      -
      -
      -

    * - :cpp:class:`roq::TradeSummary`
      -
      -
      -

    * - :cpp:class:`roq::StatisticsUpdate`
      -
      -
      -


Statistics
~~~~~~~~~~

.. list-table::
  :header-rows: 1
  :widths: auto

  * - Type
    - Comments

  * - :cpp:class:`SETTLEMENT_PRICE`
    - (instrument) :code:`markPrice`

  * - :cpp:class:`OPEN_INTEREST`
    - (instrument) :code:`openInterest`

  * - :cpp:class:`PRE_SETTLEMENT_PRICE`
    - (instrument) :code:`indicativeSettlePrice`

  * - :cpp:class:`UPPER_LIMIT_PRICE`
    - (instrument) :code:`limitUpPrice`

  * - :cpp:class:`LOWER_LIMIT_PRICE`
    - (instrument) :code:`limitDownPrice`

  * - :cpp:class:`INDEX_VALUE`
    - (instrument) :code:`fairPrice`

  * - :cpp:class:`FUNDING_RATE`
    - (funding) :code:`fundingRate`

  * - :cpp:class:`FUNDING_RATE_PREDICTION`
    - (funding) :code:`indicativeFundingRate`


Order Management
----------------

.. tab:: Live

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::OrderUpdate`
      - DropCopy
      - execution
      -

    * - :cpp:class:`roq::TradeUpdate`
      - DropCopy
      - execution
      -

.. tab:: Download

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::OrderUpdate`
      - OrderEntry
      - GET /api/v1/order
      -

    * - :cpp:class:`roq::TradeUpdate`
      - OrderEntry
      - GET /api/v1/trade
      -

.. tab:: Request

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::CreateOrder`
      - OrderEntry
      - POST /api/v1/order
      -

    * - :cpp:class:`roq::ModifyOrder`
      - OrderEntry
      - PUT /api/v1/order
      -

    * - :cpp:class:`roq::CancelOrder`
      - OrderEntry
      - DELETE /api/v1/order
      -

    * - :cpp:class:`roq::CancelAllOrders`
      - OrderEntry
      - DELETE /api/v1/order/all
      -

.. tab:: Response

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::OrderAck`
      - OrderEntry
      - /api/v1/order
      - Errors, only


Order Types
~~~~~~~~~~~

.. list-table::
  :header-rows: 1
  :widths: auto

  * - Type
    - Comments

  * - :cpp:class:`MARKET`
    - Mapped to :code:`Market` (JSON)

  * - :cpp:class:`LIMIT`
    - Mapped to :code:`Limit` (JSON)


Time in Force
~~~~~~~~~~~~~

.. list-table::
  :header-rows: 1
  :widths: auto

  * - Type
    - Comments

  * - :cpp:class:`GTC`
    - Mapped to :code:`GoodTillCancel` (JSON)



Position Effect
~~~~~~~~~~~~~~~

.. note::

  Not supported


Execution Instructions
~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
  :header-rows: 1
  :widths: auto

  * - Type
    - Comments

  * - :cpp:class:`PARTICIPATE_DO_NOT_INITIATE`
    - Mapped to :code:`ParticipateDoNotInitiate` (JSON)

  * - :cpp:class:`DO_NOT_INCREASE`
    - Mapped to :code:`ReduceOnly` (JSON)


Account Management
------------------

.. tab:: Live

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::PositionUpdate`
      - DropCopy
      - position
      -

    * - :cpp:class:`roq::FundsUpdate`
      -
      -
      - Unavailable

.. tab:: Download

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::PositionUpdate`
      - OrderEntry
      - GET /api/v1/position
      -

    * - :cpp:class:`roq::FundsUpdate`
      -
      -
      - Unavailable


Streams
-------

.. tab:: OrderEntry

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Type
      - Comments

    * - REST
      - Primary purpose

        * support order management

        Each connection

        * supports a single account

        Rate-limit avoidance is implemented by

        * **not** using HTTP pipelining
        * monitoring the following HTTP headers

        * :code:`x-ratelimit-limit`
        * :code:`x-ratelimit-remaining`
        * :code:`x-ratelimit-reset`

        * blocking new requests for a duration of
          :code:`--rest_rate_limit_interval`
          after receiving HTTP status 429 (too many requests)

.. tab:: DropCopy

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Type
      - Comments

    * - WebSocket
      - Primary purpose

        * live account updates, including orders and fills

        Each connection

        * supports a single account

.. tab:: MarketData

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Type
      - Comments

    * - WebSocket
      - Primary purpose

        * live market data

        A single connection


Constraints
-----------

* The field :code:`clOrdID` is a string and can not exceed 36 characters

Comments
--------

* The exchange API's do not appear particularly suitable for low latency trading:

  * Order action requests must be signed (which is expensive) and then sent over REST
  * REST response (order ack) could be lost possibly leaving the client in a situation where it
    must trigger timeout logic and/or issue operational alerts
  * Several WebSocket channels are used communicate order state possibly allowing for inconsistent
    order management by client
