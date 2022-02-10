#include "py_xtp_quote.h"
#include "xquote_api_struct.h"
#include "xtp_api_data_type.h"
#include "xtp_api_struct_common.h"
#include "xtp_quote_api.h"
#include <cstring>
#include <string>

inline void get_str(py::dict &d, const std::string &key, char *value)
{
    if (d.contains(key)) {
        auto v = d[key.c_str()];
        std::string v_s(v.str());
        auto buf = v_s.c_str();
        std::strncpy(value, buf, strlen(buf) + 1);
    }
}

void QuoteApi::OnError(XTPRI *error_info)
{
    XTPRI error{};
    error.error_id = 0;
    if (error_info != nullptr)
        error = *error_info;

     on_error(error);
}

void QuoteApi::on_error(const XTPRI& error)
{
    PyLock lock;

    py::dict error_info;

    error_info["error_id"] = error.error_id;
    error_info["error_msg"] = error.error_msg;

    on_error(error_info);
}

void QuoteApi::OnDepthMarketData(XTPMD *market_data, int64_t bid1_qty[], int32_t bid1_count, int32_t max_bid1_count,
                                 int64_t ask1_qty[], int32_t ask1_count, int32_t max_ask1_count)
{
    PyLock lock;

    auto data = *market_data;
    py::list bid1_qty_queue, ask1_qty_queue;

    if (bid1_qty && bid1_count > 0) {
        for (int i = 0; i < bid1_count; i++) {
            bid1_qty_queue.append(bid1_qty[i]);
        }
    }

    if (ask1_qty && ask1_count > 0) {
        for (int i = 0; i < ask1_count; i++) {
            ask1_qty_queue.append(ask1_qty[i]);
        }
    }

    process_depth_market_data(data, bid1_qty_queue, bid1_count, max_bid1_count, ask1_qty_queue, ask1_count,
                              max_ask1_count);
}

inline void QuoteApi::process_depth_market_data(
        const XTPMD &data,
        py::list& bid1_qty_queue,
        int32_t bid1_count,
        int32_t max_bid1_count,
        py::list& ask1_qty_queue,
        int32_t ask1_count,
        int32_t max_ask1_count)
{
    py::dict market_data;

    market_data["exchange_id"] = static_cast<int>(data.exchange_id);
    market_data["ticker"] = std::string(data.ticker);
    market_data["last_price"] = data.last_price;
    market_data["pre_close_price"] = data.pre_close_price;

    market_data["open_price"] = data.open_price;
    market_data["high_price"] = data.high_price;
    market_data["low_price"] = data.low_price;
    market_data["close_price"] = data.close_price;

    market_data["pct_chg"] = (data.last_price - data.upper_limit_price) / data.upper_limit_price * 100;
    market_data["pre_total_long_position"] = data.pre_total_long_positon;
    market_data["total_long_position"] = data.total_long_positon;
    market_data["pre_settl_price"] = data.pre_settl_price;

    market_data["upper_limit_price"] = data.upper_limit_price;
    market_data["lower_limit_price"] = data.lower_limit_price;
    market_data["pre_delta"] = data.pre_delta;
    market_data["curr_delta"] = data.curr_delta;

    market_data["data_time"] = data.data_time;

    market_data["qty"] = data.qty;
    market_data["turnover"] = data.turnover;
    market_data["avg_price"] = data.avg_price;

    market_data["trades_count"] = data.trades_count;

    char str_ticker_status[9] = {'\0'};
    std::strncpy(str_ticker_status, data.ticker_status, sizeof(data.ticker_status));
    market_data["ticker_status"] = std::string(str_ticker_status);

    py::list ask, bid, ask_qty, bid_qty;
    for (int i = 0; i < 10; i++) {
        ask.append(data.ask[i]);
        bid.append(data.bid[i]);
        ask_qty.append(data.ask_qty[i]);
        bid_qty.append(data.bid_qty[i]);
    }

    market_data["ask"] = ask;
    market_data["bid"] = bid;
    market_data["ask_qty"] = ask_qty;
    market_data["bid_qty"] = bid_qty;

    market_data["data_type"] = static_cast<int>(data.data_type);
    if (data.data_type == XTP_MARKETDATA_ACTUAL) {
        market_data["total_bid_qty"] = data.stk.total_bid_qty;
        market_data["total_ask_qty"] = data.stk.total_ask_qty;
        market_data["ma_bid_price"] = data.stk.ma_bid_price;
        market_data["ma_ask_price"] = data.stk.ma_ask_price;
        market_data["ma_bond_bid_price"] = data.stk.ma_bond_bid_price;
        market_data["ma_bond_ask_price"] = data.stk.ma_bond_ask_price;
        market_data["yield_to_maturity"] = data.stk.yield_to_maturity;
        market_data["iopv"] = data.stk.iopv;
        market_data["etf_buy_count"] = data.stk.etf_buy_count;
        market_data["etf_sell_count"] = data.stk.etf_sell_count;
        market_data["etf_buy_qty"] = data.stk.etf_buy_qty;
        market_data["etf_buy_money"] = data.stk.etf_buy_money;
        market_data["etf_sell_qty"] = data.stk.etf_sell_qty;
        market_data["etf_sell_money"] = data.stk.etf_sell_money;
        market_data["total_warrant_exec_qty"] = data.stk.total_warrant_exec_qty;
        market_data["warrant_lower_price"] = data.stk.warrant_lower_price;
        market_data["warrant_upper_price"] = data.stk.warrant_upper_price;
        market_data["cancel_buy_count"] = data.stk.cancel_buy_count;
        market_data["cancel_sell_count"] = data.stk.cancel_sell_count;
        market_data["cancel_buy_qty"] = data.stk.cancel_buy_qty;
        market_data["cancel_sell_qty"] = data.stk.cancel_sell_qty;
        market_data["cancel_buy_money"] = data.stk.cancel_buy_money;
        market_data["cancel_sell_money"] = data.stk.cancel_sell_money;
        market_data["total_buy_count"] = data.stk.total_buy_count;
        market_data["total_sell_count"] = data.stk.total_sell_count;
        market_data["duration_after_buy"] = data.stk.duration_after_buy;
        market_data["duration_after_sell"] = data.stk.duration_after_sell;
        market_data["num_bid_orders"] = data.stk.num_bid_orders;
        market_data["num_ask_orders"] = data.stk.num_ask_orders;
        market_data["pre_iopv"] = data.stk.pre_iopv;
        market_data["r1"] = data.stk.r1;
        market_data["r2"] = data.stk.r2;
    } else if (data.data_type == XTP_MARKETDATA_OPTION) {
        market_data["auction_price"] = data.opt.auction_price;
        market_data["auction_qty"] = data.opt.auction_qty;
        market_data["last_enquiry_time"] = data.opt.last_enquiry_time;
    }
    market_data["r4"] = data.r4;

    on_depth_market_data(
            market_data,
            bid1_qty_queue,
            bid1_count,
            max_bid1_count,
            ask1_qty_queue,
            ask1_count,
            max_ask1_count
    );
}

void QuoteApi::OnSubscribeAllMarketData(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info)
{
    XTPRI error{};
    error.error_id = 0;
    if (error_info != nullptr)
        error = *error_info;

    process_subscribe_all_market_data(exchange_id, error);
}

inline void QuoteApi::process_subscribe_all_market_data(XTP_EXCHANGE_TYPE exchange_id, const XTPRI &error)
{
    PyLock lock;

    py::dict error_info;

    error_info["error_id"] = error.error_id;
    error_info["error_msg"] = error.error_msg;

    on_subscribe_all_market_data(static_cast<int>(exchange_id), error_info);
}

void QuoteApi::OnSubMarketData(XTPST *ticker, XTPRI *error_info, bool is_last)
{
    auto data = *ticker;
    XTPRI error{};
    error.error_id = 0;
    if (error_info != nullptr)
        error = *error_info;

    process_sub_market_data(data, error, is_last);
}

inline void QuoteApi::process_sub_market_data(const XTPST &data, const XTPRI &error, bool is_last)
{
    PyLock lock;

    py::dict ticker;
    py::dict error_info;

    ticker["exchange_id"] = static_cast<int>(data.exchange_id);
    ticker["ticker"] = std::string(data.ticker);

    error_info["error_id"] = error.error_id;
    error_info["error_msg"] = error.error_msg;

    on_sub_market_data(ticker, error_info, is_last);
}

void QuoteApi::OnQueryTickersPriceInfo(XTPTPI *ticker_info, XTPRI *error_info, bool is_last)
{
    auto data = *ticker_info;
    XTPRI error{};
    error.error_id = 0;
    if (error_info != nullptr)
        error = *error_info;

    process_query_tickers_price_info(data, error, is_last);
}

inline void QuoteApi::process_query_tickers_price_info(const XTPTPI &data, const XTPRI &error, bool is_last)
{
    PyLock lock;

    py::dict ticker_info;
    py::dict error_info;

    ticker_info["exchange_id"] = static_cast<int>(data.exchange_id);
    ticker_info["ticker"] = std::string(data.ticker);
    ticker_info["last_price"] = data.last_price;

    error_info["error_id"] = error.error_id;
    error_info["error_msg"] = error.error_msg;

    on_query_tickers_price_info(ticker_info, error_info, is_last);
}

void QuoteApi::OnQueryAllTickersFullInfo(XTPQFI *ticker_info, XTPRI *error_info, bool is_last)
{
    auto data = *ticker_info;

    XTPRI error{};
    error.error_id = 0;
    if (error_info != nullptr)
        error = *error_info;

    process_query_all_tickers_full_info(data, error, is_last);
}

inline void QuoteApi::process_query_all_tickers_full_info(const XTPQFI &data, const XTPRI &error, bool is_last)
{
    PyLock lock;

    py::dict tickers_info;
    py::dict error_info;

    tickers_info["exchange_id"] = static_cast<int>(data.exchange_id);
    tickers_info["ticker"] = std::string(data.ticker);
    tickers_info["ticker_name"] = std::string(data.ticker_name);
    tickers_info["security_type"] = static_cast<int>(data.security_type);
    tickers_info["ticker_qualification_class"] = static_cast<int>(data.ticker_qualification_class);
    tickers_info["is_registration"] = data.is_registration;
    tickers_info["is_vie"] = data.is_VIE;
    tickers_info["is_no_profit"] = data.is_noprofit;
    tickers_info["is_weighted_voting_rights"] = data.is_weighted_voting_rights;
    tickers_info["is_have_price_limit"] = data.is_have_price_limit;
    tickers_info["upper_limit_price"] = data.upper_limit_price;
    tickers_info["lower_limit_price"] = data.lower_limit_price;
    tickers_info["pre_close_price"] = data.pre_close_price;
    tickers_info["price_tick"] = data.price_tick;
    tickers_info["bid_qty_upper_limit"] = data.bid_qty_upper_limit;
    tickers_info["bid_qty_lower_limit"] = data.bid_qty_lower_limit;
    tickers_info["bid_qty_unit"] = data.bid_qty_unit;
    tickers_info["ask_qty_upper_limit"] = data.ask_qty_upper_limit;
    tickers_info["ask_qty_lower_limit"] = data.ask_qty_lower_limit;
    tickers_info["ask_qty_unit"] = data.ask_qty_unit;
    tickers_info["market_bid_qty_upper_limit"] = data.market_bid_qty_upper_limit;
    tickers_info["market_bid_qty_lower_limit"] = data.market_bid_qty_lower_limit;
    tickers_info["market_bid_qty_unit"] = data.market_bid_qty_unit;
    tickers_info["market_ask_qty_upper_limit"] = data.market_ask_qty_upper_limit;
    tickers_info["market_ask_qty_lower_limit"] = data.market_ask_qty_lower_limit;
    tickers_info["market_ask_qty_unit"] = data.market_ask_qty_unit;
    tickers_info["security_status"] = static_cast<int>(data.security_status);

    error_info["error_id"] = error.error_id;
    error_info["error_msg"] = error.error_msg;

    on_query_all_tickers_full_info(tickers_info, error_info, is_last);
}

int QuoteApi::login(std::string ip, int port, std::string user, std::string passwd, int socktype, std::string local_ip)
{
    return api_ptr_->Login(ip.c_str(), port, user.c_str(), passwd.c_str(), (XTP_PROTOCOL_TYPE) socktype,
                           local_ip.c_str());
}

int QuoteApi::logout()
{ return api_ptr_->Logout(); }

int QuoteApi::subscribe_all_tick_by_tick(int exchange)
{ return api_ptr_->SubscribeAllTickByTick((XTP_EXCHANGE_TYPE) exchange); }

int QuoteApi::unsubscribe_all_tick_by_tick(int exchange)
{ return api_ptr_->UnSubscribeAllTickByTick((XTP_EXCHANGE_TYPE) exchange); }

int QuoteApi::subscribe_all_market_data(int exchange)
{ return api_ptr_->SubscribeAllMarketData((XTP_EXCHANGE_TYPE) exchange); }

int QuoteApi::unsubscribe_all_market_data(int exchange)
{ return api_ptr_->UnSubscribeAllMarketData((XTP_EXCHANGE_TYPE) exchange); }

int QuoteApi::subscribe_market_data(py::list ticker_list, int count, int exchange)
{
    auto list_len = py::len(ticker_list);
    if (list_len <= 0) return -1;
    char **tickers = new char *[list_len];
    for (size_t i = 0; i < list_len; i++) {
        tickers[i] = new char[256];
        py::dict req = (py::dict) ticker_list[i];
        get_str(req, "ticker", tickers[i]);
    }
    auto s = api_ptr_->SubscribeMarketData(tickers, count, (XTP_EXCHANGE_TYPE) exchange);
    delete[] tickers;
    tickers = nullptr;

    return s;
}

int QuoteApi::unsubscribe_market_data(py::list ticker_list, int count, int exchange)
{
    auto list_len = py::len(ticker_list);
    if (list_len <= 0) return -1;
    char **tickers = new char *[list_len];
    for (size_t i = 0; i < list_len; i++) {
        tickers[i] = new char[256];
        py::dict req = (py::dict) ticker_list[i];
        get_str(req, "ticker", tickers[i]);
    }

    auto s = api_ptr_->UnSubscribeMarketData(tickers, count, (XTP_EXCHANGE_TYPE) exchange);
    delete[] tickers;
    tickers = nullptr;

    return s;
}

int QuoteApi::query_all_tickers_full_info(int exchange)
{ return api_ptr_->QueryAllTickersFullInfo((XTP_EXCHANGE_TYPE) exchange); }

int QuoteApi::query_tickers_price_info(py::list ticker_list, int count, int exchange)
{
    auto list_len = py::len(ticker_list);
    if (list_len <= 0) return -1;
    char **tickers = new char *[list_len];
    for (size_t i = 0; i < list_len; i++) {
        tickers[i] = new char[256];
        py::dict req = (py::dict) ticker_list[i];
        get_str(req, "ticker", tickers[i]);
    }

    auto s = api_ptr_->QueryTickersPriceInfo(tickers, count, (XTP_EXCHANGE_TYPE) exchange);
    delete[] tickers;
    tickers = nullptr;

    return s;
}

int QuoteApi::query_all_tickers_price_info()
{
    return api_ptr_->QueryAllTickersPriceInfo();
}

void QuoteApi::create_quote_api(int client_id, std::string path, int log_level)
{
    api_ptr_ = XTP::API::QuoteApi::CreateQuoteApi(client_id, path.c_str(), (XTP_LOG_LEVEL) log_level);
    api_ptr_->RegisterSpi(this);
}

void QuoteApi::release()
{
    api_ptr_->Release();
}

int QuoteApi::exit()
{
    // 该函数在原生API里没有,用于安全退出API用,原生的join似乎不太稳定
    api_ptr_->RegisterSpi(nullptr);
    api_ptr_->Release();
    api_ptr_ = nullptr;
    return 1;
}

std::string QuoteApi::get_trading_day()
{
    auto p = api_ptr_->GetTradingDay();
    return (p == nullptr) ? "NULL" : p;
}

std::string QuoteApi::get_api_version()
{
    auto p = api_ptr_->GetApiVersion();
    return (p == nullptr) ? "NULL" : p;
}

py::dict QuoteApi::get_api_last_error()
{
    auto error = api_ptr_->GetApiLastError();

    py::dict err;
    if (error == nullptr)
        return err;

    err["error_id"] = error->error_id;
    err["error_msg"] = error->error_msg;

    return err;
}

struct QuoteApiWrap : QuoteApi
{
    void on_disconnected(int reason) override
    {
        try {
            pybind11::get_overload(this, "on_disconnected")(reason);
        } catch (pybind11::error_already_set const &) {
            PyErr_Print();
        }
    }

    void on_error(py::dict data) override
    {
        try {
            pybind11::get_overload(this, "on_error")(data);
        } catch (pybind11::error_already_set const &) {
            PyErr_Print();
        }
    }

    void on_sub_market_data(py::dict data, py::dict error, bool last) override
    {
        try {
            pybind11::get_overload(this, "on_sub_market_data")(data, error, last);
        } catch (pybind11::error_already_set const &) {
            PyErr_Print();
        }
    }

    void on_unsub_market_data(py::dict data, py::dict error, bool last) override
    {
        try {
            pybind11::get_overload(this, "on_unsub_market_data")(data, error, last);
        } catch (pybind11::error_already_set const &) {
            PyErr_Print();
        }
    }

    void on_depth_market_data(py::dict data, py::list bid1_qty_list, int bid1_count, int max_bid1_count,
                              py::list ask1_qty_list, int ask1_count, int max_ask1_count) override
    {
        try {
            pybind11::get_overload(this, "on_depth_market_data")(data, bid1_qty_list, bid1_count, max_bid1_count,
                                                                 ask1_qty_list, ask1_count, max_ask1_count);
        } catch (pybind11::error_already_set const &) {
            PyErr_Print();
        }
    }

    void on_query_all_tickers_full_info(py::dict data, py::dict error, bool last) override
    {
        try {
            pybind11::get_overload(this, "on_query_all_tickers_full_info")(data, error, last);
        } catch (pybind11::error_already_set const &) {
            PyErr_Print();
        }
    }

    void on_query_tickers_price_info(py::dict data, py::dict error, bool last) override
    {
        try {
            pybind11::get_overload(this, "on_query_tickers_price_info")(data, error, last);
        } catch (pybind11::error_already_set const &) {
            PyErr_Print();
        }
    }

    void on_subscribe_all_market_data(int exchange_id, py::dict error) override
    {
        try {
            pybind11::get_overload(this, "on_subscribe_all_market_data")(exchange_id, error);
        } catch (pybind11::error_already_set const &) {
            PyErr_Print();
        }
    }

};

PYBIND11_MODULE(py_xtp_quote, m)
{
//    PyEval_InitThreads();
    m.doc() = "XTP SDK(v1.2.5)";

    py::class_<QuoteApiWrap>(m, "QuoteApi")
            .def(py::init<>())
            .def(
                    "get_api_version",
                    &QuoteApiWrap::get_api_version,
                    "获取API的发行版本号"
            )
            .def(
                    "get_trading_day",
                    &QuoteApiWrap::get_trading_day,
                    "获取当前交易日"
            )
            .def(
                    "get_api_last_error",
                    &QuoteApiWrap::get_api_last_error,
                    "获取API的系统错误,可以在Login、Logout、订阅、取消订阅失败时调用,获取失败的原因"
            )
            .def(
                    "create_quote_api",
                    &QuoteApiWrap::create_quote_api,
                    "创建QuoteApi,返回创建出的UserApi",
                    py::arg("client_id"),
                    py::arg("log_path"),
                    py::arg("log_level")
            )
            .def(
                    "query_all_tickers_full_info",
                    &QuoteApiWrap::query_all_tickers_full_info,
                    "获取所有合约的详细静态信息,包括指数等非可交易的,“0”表示发送查询请求成功,非“0”表示发送查询请求不成功",
                    py::arg("exchange_id")
            )
            .def(
                    "login",
                    &QuoteApiWrap::login,
                    "用户登录请求,“0”表示登录成功,“-1”表示连接服务器出错,此时用户可以调用GetApiLastError()来获取错误代码,“-2”表示已存在连接,不允许重复登录,如果需要重连,请先logout,“-3”表示输入有错误",
                    py::arg("ip"),
                    py::arg("port"),
                    py::arg("user"),
                    py::arg("passwd"),
                    py::arg("socktype"),
                    py::arg("local_ip")
            )
            .def(
                    "logout",
                    &QuoteApiWrap::logout,
                    "登出请求,“0”表示登出成功,非“0”表示登出出错,此时用户可以调用GetApiLastError()来获取错误代码"
            )
            .def(
                    "query_all_tickers_price_info",
                    &QuoteApiWrap::query_all_tickers_price_info,
                    "获取所有合约的最新价格信息,“0”表示发送查询请求成功,非“0”表示发送查询请求不成功"
            )
            .def(
                    "query_tickers_price_info",
                    &QuoteApiWrap::query_tickers_price_info,
                    "获取合约的最新价格信息,“0”表示发送查询请求成功,非“0”表示发送查询请求不成功",
                    py::arg("ticker"),
                    py::arg("count"),
                    py::arg("exchange_id")
            )
            .def(
                    "subscribe_market_data",
                    &QuoteApiWrap::subscribe_market_data,
                    "订阅行情,包括股票、指数和期权,“0”表示接口调用成功,非“0”表示接口调用出错",
                    py::arg("ticker"),
                    py::arg("count"),
                    py::arg("exchange_id")
            )
            .def(
                    "unsubscribe_market_data",
                    &QuoteApiWrap::unsubscribe_market_data,
                    "退订行情,包括股票、指数和期权, “0”表示接口调用成功,非“0”表示接口调用出错",
                    py::arg("ticker"),
                    py::arg("count"),
                    py::arg("exchange_id")
            )
            .def(
                    "subscribe_all_market_data",
                    &QuoteApiWrap::subscribe_all_market_data,
                    "订阅全市场的股票行情",
                    py::arg("exchange_id")
            )
            .def(
                    "unsubscribe_all_market_data",
                    &QuoteApiWrap::unsubscribe_all_market_data,
                    "退订全市场的股票行情",
                    py::arg("exchange_id")
            )
            .def(
                    "subscribe_all_tick_by_tick",
                    &QuoteApiWrap::subscribe_all_tick_by_tick,
                    "订阅全市场的股票逐笔行情,“0”表示接口调用成功,非“0”表示接口调用出错",
                    py::arg("exchange_id")
            )
            .def(
                    "unsubscribe_all_tick_by_tick",
                    &QuoteApiWrap::unsubscribe_all_tick_by_tick,
                    "退订全市场的股票逐笔行情,“0”表示接口调用成功,非“0”表示接口调用出错",
                    py::arg("exchange_id")
            )
            .def(
                    "process_depth_market_data",
                    &QuoteApiWrap::on_depth_market_data,
                    "深度行情通知,包含买一卖一队列",
                    py::arg("market_data"),
                    py::arg("bid1_qty"),
                    py::arg("bid1_count"),
                    py::arg("max_bid1_count"),
                    py::arg("ask1_qty"),
                    py::arg("ask1_count"),
                    py::arg("max_ask1_count")
            )
            .def(
                    "on_disconnected",
                    &QuoteApiWrap::on_disconnected,
                    "当客户端与行情后台通信连接断开时,该方法被调用",
                    py::arg("reason")
            )
            .def(
                    "on_error",
                    &QuoteApiWrap::on_error,
                    "错误应答",
                    py::arg("error_info")
            )
            .def(
                    "on_sub_market_data",
                    &QuoteApiWrap::on_sub_market_data,
                    "订阅行情应答,包括股票、指数和期权",
                    py::arg("ticker"),
                    py::arg("error_info"),
                    py::arg("is_last")
            )
            .def(
                    "on_query_all_tickers",
                    &QuoteApiWrap::on_query_all_tickers,
                    "查询合约部分静态信息的应答",
                    py::arg("ticker_info"),
                    py::arg("error_info"),
                    py::arg("is_last")
            )
            .def(
                    "on_query_tickers_price_info",
                    &QuoteApiWrap::on_query_tickers_price_info,
                    "查询合约的最新价格信息应答",
                    py::arg("ticker_info"),
                    py::arg("error_info"),
                    py::arg("is_last")
            )
            .def(
                    "on_query_all_tickers_full_info",
                    &QuoteApiWrap::on_query_all_tickers_full_info,
                    "查询合约完整静态信息的应答",
                    py::arg("ticker_info"),
                    py::arg("error_info"),
                    py::arg("is_last")
            )
            .def(
                    "process_subscribe_all_market_data",
                    &QuoteApi::on_subscribe_all_market_data,
                    "订阅全市场的股票行情应答",
                    py::arg("exchange_id"),
                    py::arg("error_info")
            );
}

