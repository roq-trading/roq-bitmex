.. _roq-bitmex:

.. |dagger| unicode:: U+2020
.. |double-dagger| unicode:: U+2021
.. |right-arrow| unicode:: U+2192
.. |right-double-arrow| unicode:: U+21D2
.. |left-right-double-arrow| unicode:: U+21D4
.. |check-mark| unicode:: U+2705
.. |cross-mark| unicode:: U+274C
.. |negative-cross-mark| unicode:: U+274E
.. |footnote-1| unicode:: U+2776
.. |footnote-2| unicode:: U+2777
.. |footnote-3| unicode:: U+2778


roq-bitmex
==========


.. tab:: Unstable

  .. code-block:: shell

     $ conda install \
           --channel https://roq-trading.com/conda/unstable \
           roq-bitmex

.. tab:: Stable

  .. code-block:: shell

     $ conda install \
           --channel https://roq-trading.com/conda/stable \
           roq-bitmex


Supports
--------

.. grid::  2
  :gutter: 2

  .. grid-item-card::  Products

    .. list-table::
      :widths: auto
      :align: left

      * - :cpp:enumerator:`Spot <roq::SecurityType::SPOT>`
        - |check-mark|
        -
      * - :cpp:enumerator:`Futures <roq::SecurityType::FUTURES>`
        - |check-mark|
        -
      * - :cpp:enumerator:`Swap <roq::SecurityType::SWAP>`
        - |check-mark|
        -
      * - :cpp:enumerator:`Option <roq::SecurityType::OPTION>`
        - |cross-mark|
        -

  .. grid-item-card::  Market Data

    .. list-table::
      :widths: auto
      :align: left

      * - :cpp:class:`ReferenceData <roq::ReferenceData>`
        - |check-mark|
        -
      * - :cpp:class:`MarketStatus <roq::MarketStatus>`
        - |check-mark|
        -
      * - :cpp:class:`TopOfBook <roq::TopOfBook>`
        - |check-mark|
        -
      * - :cpp:class:`MarketByPrice <roq::MarketByPriceUpdate>`
        - |check-mark|
        -
      * - :cpp:class:`MarketByOrder <roq::MarketByOrderUpdate>`
        - |cross-mark|
        -
      * - :cpp:class:`TradeSummary <roq::TradeSummary>`
        - |check-mark|
        -
      * - :cpp:class:`Statistics <roq::StatisticsUpdate>`
        - |check-mark|
        -
      * - :cpp:class:`TimeSeries <roq::TimeSeriesUpdate>`
        - |cross-mark|
        -

  .. grid-item-card::  Orders & Quotes

    .. list-table::
      :widths: auto
      :align: left

      * - :cpp:class:`CreateOrder <roq::CreateOrder>`
        - |check-mark|
        -
      * - :cpp:class:`ModifyOrder <roq::ModifyOrder>`
        - |check-mark|
        -
      * - :cpp:class:`CancelOrder <roq::CancelOrder>`
        - |check-mark|
        -
      * - :cpp:class:`CancelAllOrders <roq::CancelAllOrders>`
        - |check-mark|
        -
      * - :cpp:class:`MassQuote <roq::MassQuote>`
        - |cross-mark|
        -
      * - :cpp:class:`CancelQuotes <roq::CancelQuotes>`
        - |cross-mark|
        -

  .. grid-item-card::  Account

    .. list-table::
      :widths: auto
      :align: left

      * - :cpp:class:`Funds <roq::FundsUpdate>`
        - |cross-mark|
        -
      * - :cpp:class:`Position <roq::PositionUpdate>`
        - |check-mark|
        -

.. note::

   |check-mark| = Available.

   |negative-cross-mark| = Not implemented.

   |cross-mark| = Unavailable.


Using
-----

.. code-block:: shell

   $ roq-bitmex [FLAGS]


.. _roq-bitmex-flags:

Flags
-----

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

      $ --flagfile $CONDA_PREFIX/share/roq-bitmex/flags/prod/flags.cfg

   .. include:: flags/prod/flags.cfg
     :code: shell

.. tab:: Test

   .. code-block:: shell

      $ --flagfile $CONDA_PREFIX/share/roq-bitmex/flags/test/flags.cfg

   .. include:: flags/test/flags.cfg
     :code: shell


Configuration
-------------

.. code-block:: shell

   $ --flagfile $CONDA_PREFIX/share/roq-bitmex/config.toml

.. important::

   This template will be replaced when the software is upgraded.
   Make a copy and modify to your own needs.


.. include:: config.toml
   :code: toml


Market Data
-----------


Inbound
~~~~~~~

.. tab:: TradingStatus

   .. list-table::
     :header-rows: 1
     :widths: auto
     :align: left

     * - :code:`state`
       -
       -

     * - :code:`Open`
       - |right-double-arrow|
       - :cpp:enumerator:`OPEN <roq::TradingStatus::OPEN>`

     * - :code:`Closed`
       - |right-double-arrow|
       - :cpp:enumerator:`CLOSE <roq::TradingStatus::CLOSE>`

     * - :code:`Settled`
       - |right-double-arrow|
       - :cpp:enumerator:`UNDEFINED <roq::TradingStatus::UNDEFINED>`

     * - :code:`Unlisted`
       - |right-double-arrow|
       - :cpp:enumerator:`UNDEFINED <roq::TradingStatus::UNDEFINED>`

     * - :code:`Expired`
       - |right-double-arrow|
       - :cpp:enumerator:`UNDEFINED <roq::TradingStatus::UNDEFINED>`


.. tab:: StatisticsType

   .. list-table::
     :header-rows: 1
     :widths: auto
     :align: left

     * - Table
       - Field
       -
       -

     * - :code:`instrument`
       - :code:`markPrice`
       - |right-double-arrow|
       - :cpp:enumerator:`SETTLEMENT_PRICE <roq::StatisticsType::SETTLEMENT_PRICE>`

     * - :code:`instrument`
       - :code:`openInterest`
       - |right-double-arrow|
       - :cpp:enumerator:`OPEN_INTEREST <roq::StatisticsType::OPEN_INTEREST>`

     * - :code:`instrument`
       - :code:`indicativeSettlePrice`
       - |right-double-arrow|
       - :cpp:enumerator:`PRE_SETTLEMENT_PRICE <roq::StatisticsType::PRE_SETTLEMENT_PRICE>`

     * - :code:`instrument`
       - :code:`limitUpPrice`
       - |right-double-arrow|
       - :cpp:enumerator:`UPPER_LIMIT_PRICE <roq::StatisticsType::UPPER_LIMIT_PRICE>`

     * - :code:`instrument`
       - :code:`limitDownPrice`
       - |right-double-arrow|
       - :cpp:enumerator:`LOWER_LIMIT_PRICE <roq::StatisticsType::LOWER_LIMIT_PRICE>`

     * - :code:`instrument`
       - :code:`fairPrice`
       - |right-double-arrow|
       - :cpp:enumerator:`INDEX_VALUE <roq::StatisticsType::INDEX_VALUE>`

     * - :code:`funding`
       - :code:`fundingRate`
       - |right-double-arrow|
       - :cpp:enumerator:`FUNDING_RATE <roq::StatisticsType::FUNDING_RATE>`

     * - :code:`funding`
       - :code:`indicativeFundingRate`
       - |right-double-arrow|
       - :cpp:enumerator:`FUNDING_RATE_PREDICTION <roq::StatisticsType::FUNDING_RATE_PREDICTION>`


Order Management
----------------



Inbound
~~~~~~~

.. tab:: OrderType

   .. list-table::
     :header-rows: 1
     :widths: auto
     :align: left

     * - Enum
       -
       -

     * - :code:`Market`
       - |right-double-arrow|
       - :cpp:enumerator:`MARKET <roq::OrderType::MARKET>`

     * - :code:`Limit`
       - |right-double-arrow|
       - :cpp:enumerator:`LIMIT <roq::OrderType::LIMIT>`


.. tab:: TimeInForce

   .. list-table::
     :header-rows: 1
     :widths: auto
     :align: left

     * - Enum
       -
       -

     * - :code:`GTC`
       - |right-double-arrow|
       - :cpp:enumerator:`GTC <roq::TimeInForce::GTC>`


.. tab:: OrderStatus

   .. list-table::
     :header-rows: 1
     :widths: auto
     :align: left

     * - Enum
       -
       -

     * - :code:`Canceled`
       - |right-double-arrow|
       - :cpp:enumerator:`CANCELED <roq::OrderStatus::CANCELED>`

     * - :code:`DoneForDay`
       - |right-double-arrow|
       - :cpp:enumerator:`SUSPENDED <roq::OrderStatus::SUSPENDED>`

     * - :code:`Expired`
       - |right-double-arrow|
       - :cpp:enumerator:`EXPIRED <roq::OrderStatus::EXPIRED>`

     * - :code:`Filled`
       - |right-double-arrow|
       - :cpp:enumerator:`COMPLETED <roq::OrderStatus::COMPLETED>`

     * - :code:`New`
       - |right-double-arrow|
       - :cpp:enumerator:`WORKING <roq::OrderStatus::WORKING>`

     * - :code:`PartiallyFilled`
       - |right-double-arrow|
       - :cpp:enumerator:`WORKING <roq::OrderStatus::WORKING>`

     * - :code:`PendingCancel`
       - |right-double-arrow|
       - :cpp:enumerator:`UNDEFINED <roq::OrderStatus::UNDEFINED>`

     * - :code:`PendingNew`
       - |right-double-arrow|
       - :cpp:enumerator:`SENT <roq::OrderStatus::SENT>`

     * - :code:`Rejected`
       - |right-double-arrow|
       - :cpp:enumerator:`REJECTED <roq::OrderStatus::REJECTED>`

     * - :code:`Stopped`
       - |right-double-arrow|
       - :cpp:enumerator:`STOPPED <roq::OrderStatus::STOPPED>`

     * - :code:`Triggered`
       - |right-double-arrow|
       - :cpp:enumerator:`ACCEPTED <roq::OrderStatus::ACCEPTED>`

     * - :code:`Untriggered`
       - |right-double-arrow|
       - :cpp:enumerator:`WORKING <roq::OrderStatus::WORKING>`


Outbound
~~~~~~~~

.. tab:: CreateOrder

   .. list-table::
     :header-rows: 1
     :stub-columns: 1
     :widths: auto
     :align: left

     * -
       - :cpp:member:`order_type <roq::CreateOrder::order_type>`
       - :cpp:member:`execution_instructions <roq::CreateOrder::execution_instructions>`
       - :cpp:member:`price <roq::CreateOrder::price>`
       - :cpp:member:`stop_price <roq::CreateOrder::stop_price>`
       -
       - :code:`ordType`
       - :code:`price`
       - :code:`execInst`

     * -
       - :cpp:enumerator:`MARKET <roq::OrderType::MARKET>`
       -
       - :code:`NaN`
       - :code:`NaN`
       - |right-double-arrow|
       - :code:`Market`
       - |cross-mark|
       -

     * -
       - :cpp:enumerator:`MARKET <roq::OrderType::MARKET>`
       - :cpp:enumerator:`PARTICIPATE_DO_NOT_INITIATE <roq::OrderType::PARTICIPATE_DO_NOT_INITIATE>`
       - :code:`NaN`
       - :code:`NaN`
       - |right-double-arrow|
       - :code:`Market`
       - |cross-mark|
       - :code:`ParticipateDoNotInitiate`

     * -
       - :cpp:enumerator:`MARKET <roq::OrderType::MARKET>`
       - :cpp:enumerator:`DO_NOT_INCREASE <roq::OrderType::DO_NOT_INCREASE>`
       - :code:`NaN`
       - :code:`NaN`
       - |right-double-arrow|
       - :code:`Market`
       - |cross-mark|
       - :code:`ReduceOnly`

     * - |cross-mark|
       - :cpp:enumerator:`MARKET <roq::OrderType::MARKET>`
       -
       - :code:`NaN`
       - |check-mark|
       - |right-double-arrow|
       -
       -
       -

     * -
       - :cpp:enumerator:`LIMIT <roq::OrderType::LIMIT>`
       -
       - |check-mark|
       - :code:`NaN`
       - |right-double-arrow|
       - :code:`Limit`
       - |check-mark|
       -

     * -
       - :cpp:enumerator:`LIMIT <roq::OrderType::LIMIT>`
       - :cpp:enumerator:`PARTICIPATE_DO_NOT_INITIATE <roq::OrderType::PARTICIPATE_DO_NOT_INITIATE>`
       - |check-mark|
       - :code:`NaN`
       - |right-double-arrow|
       - :code:`Limit`
       - |check-mark|
       - :code:`ParticipateDoNotInitiate`

     * -
       - :cpp:enumerator:`LIMIT <roq::OrderType::LIMIT>`
       - :cpp:enumerator:`DO_NOT_INCREASE <roq::OrderType::DO_NOT_INCREASE>`
       - |check-mark|
       - :code:`NaN`
       - |right-double-arrow|
       - :code:`Limit`
       - |check-mark|
       - :code:`ReduceOnly`

     * - |cross-mark|
       - :cpp:enumerator:`LIMIT <roq::OrderType::LIMIT>`
       -
       - |check-mark|
       - |check-mark|
       -
       -
       -
       -


Comments
--------

* The field :code:`clOrdID` is a string and can not exceed 36 characters

* The exchange API's do not appear particularly suitable for low latency trading:

  * Order action requests must be signed (which is expensive) and then sent over REST
  * REST response (order ack) could be lost possibly leaving the client in a situation where it
    must trigger timeout logic and/or issue operational alerts
  * Several WebSocket channels are used communicate order state possibly allowing for inconsistent
    order management by client


References
----------


Common
~~~~~~

* :ref:`Using Conda <tutorial-conda>`
* :ref:`Using Flags <abseil-cpp>`
* :ref:`Gateway Flags <gateway-flags>`
* :ref:`Gateway Config <gateway-config>`


Exchange
~~~~~~~~

* `Website <https://www.bitmex.com/>`__
* `Testnet <https://testnet.bitmex.com/app/login>`__
* `Documentation <https://www.bitmex.com/app/apiOverview>`__
