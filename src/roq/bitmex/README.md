
fields we always receive
* account
* cl\_ord\_id
* order\_id
* symbol

create
      action  ord\_status  working  time
rest          NEW          true
ws    INSERT  NEW          false    +525us
ws    UPDATE               true     +624us








TODO

json::Instrument
- cache state and send updates
-- XXX need efficient comparison based on what has changed

	{ "name": "pegPriceType", "type": "std::string_view" },
	{ "name": "triggered", "type": "std::string_view" },

NEW ORDER

I0229 04:40:58.580676 14425 rest.cpp:306] DEBUG: body="{"orderID":"e1cc67c1-408f-456d-457c-660ee8da7a85","clOrdID":"roq-10000001-5-1001","clOrdLinkID":"","account":273093,"symbol":"XBTUSD","side":"Buy","simpleOrderQty":null,"orderQty":1,"price":8767.5,"displayQty":null,"stopPx":null,"pegOffsetValue":null,"pegPriceType":"","currency":"USD","settlCurrency":"XBt","ordType":"Limit","timeInForce":"GoodTillCancel","execInst":"","contingencyType":"","exDestination":"XBME","ordStatus":"New","triggered":"","workingIndicator":true,"ordRejReason":"","simpleLeavesQty":null,"leavesQty":1,"simpleCumQty":null,"cumQty":0,"avgPx":null,"multiLegReportingType":"SingleSecurity","text":"Submitted via API.","transactTime":"2020-02-29T04:40:58.535Z","timestamp":"2020-02-29T04:40:58.535Z"}"

I0229 04:40:58.580715 14425 rest.cpp:311] DEBUG: order_item={account=273093, avg_px=0.0, cl_ord_id="roq-10000001-5-1001", cl_ord_link_id="", contingency_type="", cum_qty=0.0, currency="USD", display_qty=0.0, ex_destination="XBME", exec_inst="", leaves_qty=1.0, multi_leg_reporting_type="SingleSecurity", order_id="e1cc67c1-408f-456d-457c-660ee8da7a85", order_qty=1.0, ord_rej_reason="", ord_status="New", ord_type="Limit", peg_offset_value=0.0, peg_price_type="", price=8767.5, settl_currency="XBt", side="Buy", simple_cum_qty=0.0, simple_leaves_qty=0.0, simple_order_qty=0.0, stop_px=0.0, symbol="XBTUSD", text="Submitted via API.", time_in_force="GoodTillCancel", timestamp=1582951258535000000ns, transact_time=1582951258535000000ns, triggered="", working_indicator=true}

I0229 04:40:58.580944 14425 gateway.cpp:233] action=INSERT order={data=[{account=273093, avg_px=0.0, cl_ord_id="roq-10000001-5-1001", cl_ord_link_id="", contingency_type="", cum_qty=0.0, currency="USD", display_qty=0.0, ex_destination="XBME", exec_inst="", leaves_qty=1.0, multi_leg_reporting_type="SingleSecurity", order_id="e1cc67c1-408f-456d-457c-660ee8da7a85", order_qty=1.0, ord_rej_reason="", ord_status="New", ord_type="Limit", peg_offset_value=0.0, peg_price_type="", price=8767.5, settl_currency="XBt", side="Buy", simple_cum_qty=0.0, simple_leaves_qty=0.0, simple_order_qty=0.0, stop_px=0.0, symbol="XBTUSD", text="Submitted via API.", time_in_force="GoodTillCancel", timestamp=1582951258535000000ns, transact_time=1582951258535000000ns, triggered="", working_indicator=false}]}

I0229 04:40:58.580964 14425 gateway.cpp:233] action=UPDATE order={data=[{account=273093, avg_px=nan, cl_ord_id="roq-10000001-5-1001", cl_ord_link_id="", contingency_type="", cum_qty=nan, currency="", display_qty=nan, ex_destination="", exec_inst="", leaves_qty=nan, multi_leg_reporting_type="", order_id="e1cc67c1-408f-456d-457c-660ee8da7a85", order_qty=nan, ord_rej_reason="", ord_status="", ord_type="", peg_offset_value=nan, peg_price_type="", price=nan, settl_currency="", side="", simple_cum_qty=nan, simple_leaves_qty=nan, simple_order_qty=nan, stop_px=nan, symbol="XBTUSD", text="", time_in_force="", timestamp=1582951258535000000ns, transact_time=0ns, triggered="", working_indicator=true}]}



DOWNLOAD


I0229 04:44:37.850504 14637 gateway.cpp:233] action=PARTIAL order={data=[{account=273093, avg_px=0.0, cl_ord_id="roq-10000001-5-1001", cl_ord_link_id="", contingency_type="", cum_qty=0.0, currency="USD", display_qty=0.0, ex_destination="XBME", exec_inst="", leaves_qty=1.0, multi_leg_reporting_type="SingleSecurity", order_id="e1cc67c1-408f-456d-457c-660ee8da7a85", order_qty=1.0, ord_rej_reason="", ord_status="New", ord_type="Limit", peg_offset_value=0.0, peg_price_type="", price=8767.5, settl_currency="XBt", side="Buy", simple_cum_qty=0.0, simple_leaves_qty=0.0, simple_order_qty=0.0, stop_px=0.0, symbol="XBTUSD", text="Submitted via API.", time_in_force="GoodTillCancel", timestamp=1582951258535000000ns, transact_time=1582951258535000000ns, triggered="", working_indicator=true}]}


// account=273093
// avg_px=0.0
// cl_ord_id="roq-10000001-5-1001"
// cl_ord_link_id=""
// contingency_type=""
// cum_qty=0.0
// currency="USD"
// display_qty=0.0
// ex_destination="XBME"
// exec_inst=""
// leaves_qty=1.0
// multi_leg_reporting_type="SingleSecurity"
// order_id="e1cc67c1-408f-456d-457c-660ee8da7a85"
// order_qty=1.0
// ord_rej_reason=""
// ord_status="New"
// ord_type="Limit"
// peg_offset_value=0.0
// peg_price_type=""
// price=8767.5
// settl_currency="XBt"
// side="Buy"
// simple_cum_qty=0.0
// simple_leaves_qty=0.0
// simple_order_qty=0.0
// stop_px=0.0
// symbol="XBTUSD"
// text="Submitted via API."
// time_in_force="GoodTillCancel"
// timestamp=1582951258535000000ns
// transact_time=1582951258535000000ns
// triggered=""
// working_indicator=false}]}










I0218 13:26:54.798018 6549 rest.cpp:304] DEBUG: body="{"orderID":"8118a623-5a03-4a78-dbae-760a5b8741ee","clOrdID":"","clOrdLinkID":"","account":273093,"symbol":"XBTUSD","side":"Buy","simpleOrderQty":null,"orderQty":1,"price":9661,"displayQty":null,"stopPx":null,"pegOffsetValue":null,"pegPriceType":"","currency":"USD","settlCurrency":"XBt","ordType":"Limit","timeInForce":"GoodTillCancel","execInst":"","contingencyType":"","exDestination":"XBME","ordStatus":"New","triggered":"","workingIndicator":true,"ordRejReason":"","simpleLeavesQty":null,"leavesQty":1,"simpleCumQty":null,"cumQty":0,"avgPx":null,"multiLegReportingType":"SingleSecurity","text":"Submitted via API.","transactTime":"2020-02-18T13:26:54.756Z","timestamp":"2020-02-18T13:26:54.756Z"}"
I0218 13:26:54.798069 6549 rest.cpp:309] DEBUG: order_item={account=273093, avg_px=0.0, cl_ord_id="", cl_ord_link_id="", contingency_type="", cum_qty=0.0, currency="USD", display_qty=0.0, ex_destination="XBME", exec_inst="", leaves_qty=1.0, multi_leg_reporting_type="SingleSecurity", ord_rej_reason="", ord_status="New", ord_type="Limit", order_id="8118a623-5a03-4a78-dbae-760a5b8741ee", order_qty=1.0, peg_offset_value=0.0, peg_price_type="", price=9661.0, settl_currency="XBt", side="Buy", simple_cum_qty=0.0, simple_leaves_qty=0.0, simple_order_qty=0.0, stop_px=0.0, symbol="XBTUSD", text="Submitted via API.", time_in_force="GoodTillCancel", timestamp=1582032414756000000ns, transact_time=1582032414756000000ns, triggered="", working_indicator=true}
I0218 13:26:54.798187 6549 gateway.cpp:255] DEBUG: action=INSERT order={data=[{account=273093, avg_px=0.0, cl_ord_id="", cl_ord_link_id="", contingency_type="", cum_qty=0.0, currency="USD", display_qty=0.0, ex_destination="XBME", exec_inst="", leaves_qty=1.0, multi_leg_reporting_type="SingleSecurity", ord_rej_reason="", ord_status="New", ord_type="Limit", order_id="8118a623-5a03-4a78-dbae-760a5b8741ee", order_qty=1.0, peg_offset_value=0.0, peg_price_type="", price=9661.0, settl_currency="XBt", side="Buy", simple_cum_qty=0.0, simple_leaves_qty=0.0, simple_order_qty=0.0, stop_px=0.0, symbol="XBTUSD", text="Submitted via API.", time_in_force="GoodTillCancel", timestamp=1582032414756000000ns, transact_time=1582032414756000000ns, triggered="", working_indicator=false}]}
I0218 13:26:54.798241 6549 gateway.cpp:255] DEBUG: action=UPDATE order={data=[{account=273093, avg_px=nan, cl_ord_id="", cl_ord_link_id="", contingency_type="", cum_qty=nan, currency="", display_qty=nan, ex_destination="", exec_inst="", leaves_qty=nan, multi_leg_reporting_type="", ord_rej_reason="", ord_status="", ord_type="", order_id="8118a623-5a03-4a78-dbae-760a5b8741ee", order_qty=nan, peg_offset_value=nan, peg_price_type="", price=nan, settl_currency="", side="", simple_cum_qty=nan, simple_leaves_qty=nan, simple_order_qty=nan, stop_px=nan, symbol="XBTUSD", text="", time_in_force="", timestamp=1582032414756000000ns, transact_time=0ns, triggered="", working_indicator=true}]}
I0218 13:26:54.814489 6549 rest.cpp:572] {"now":"2020-02-18T13:26:54.776Z","cancelTime":"2020-02-18T13:27:54.776Z"}
I0218 13:26:59.105875 6549 gateway.cpp:255] DEBUG: action=UPDATE order={data=[{account=273093, avg_px=nan, cl_ord_id="", cl_ord_link_id="", contingency_type="", cum_qty=nan, currency="", display_qty=nan, ex_destination="", exec_inst="", leaves_qty=0.0, multi_leg_reporting_type="", ord_rej_reason="", ord_status="Canceled", ord_type="", order_id="8118a623-5a03-4a78-dbae-760a5b8741ee", order_qty=nan, peg_offset_value=nan, peg_price_type="", price=nan, settl_currency="", side="", simple_cum_qty=nan, simple_leaves_qty=nan, simple_order_qty=nan, stop_px=nan, symbol="XBTUSD", text="Canceled: Timeout\nSubmitted via API.", time_in_force="", timestamp=1582032419064000000ns, transact_time=0ns, triggered="", working_indicator=false}]}


action=UPDATE order={data=[{account=273093
avg_px=nan
cl_ord_id="1"   XXXXXXXXXXX
cl_ord_link_id=""
contingency_type=""
cum_qty=nan
currency=""
display_qty=nan
ex_destination=""
exec_inst=UNDEFINED
leaves_qty=nan
multi_leg_reporting_type=UNDEFINED
order_id="f8d78a07-485f-d045-d1b1-91a67bd1da08"   XXXXXXXXXXX
order_qty=nan
ord_rej_reason=""
ord_status=UNDEFINED
ord_type=UNDEFINED
peg_offset_value=nan
peg_price_type=""
price=nan
settl_currency=""
side=UNDEFINED
simple_cum_qty=nan
simple_leaves_qty=nan
simple_order_qty=nan
stop_px=nan
symbol="XBTUSD"   XXXXXXXXXXX
text=""
time_in_force=UNDEFINED
timestamp=1583059342325000000ns   XXXXXXXXXXX
transact_time=0ns
triggered=""
working_indicator=true}]}   XXXXXXXXXXX

symbol
order_id
working
cl_ord_id
timestamp

HTTP status=BAD_REQUEST body="{"error":{"message":"Duplicate clOrdID","name":"HTTPError"}}"
