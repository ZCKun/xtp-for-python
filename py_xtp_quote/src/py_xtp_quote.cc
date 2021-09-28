#include "py_xtp_quote.h"
#include "xquote_api_struct.h"
#include "xtp_api_data_type.h"
#include "xtp_api_struct_common.h"
#include "xtp_quote_api.h"
#include <cstring>
#include <string>
#include <unistd.h>

inline void get_str(py::dict& d, const std::string& key, char* value)
{
    if (d.contains(key)) {
        auto v = d[key.c_str()];
        std::string v_s(v.str());
        auto buf = v_s.c_str();
        std::strncpy(value, buf, strlen(buf) + 1);
    }
}

inline std::string add_ending_char(char* value)
{
    return std::string(value);
}

void QuoteApi::OnError(XTPRI *error_info)
{
    auto* task = new Task();
    task->task_name = ON_ERROR;

    if (error_info) {
        auto* task_error = new XTPRI();
        *task_error = *error_info;
        task->task_error = task_error;
    }

    task_queue_.push(task);
}

void QuoteApi::OnDepthMarketData(XTPMD *market_data, int64_t bid1_qty[], int32_t bid1_count, int32_t max_bid1_count, int64_t ask1_qty[], int32_t ask1_count, int32_t max_ask1_count)
{
    // printf("%s: %f\n", market_data->ticker, market_data->last_price);
    auto *task = new Task();
    task->task_name = ON_DEPTH_MARKET_DATA;

    if (market_data) {
        auto *task_data = new XTPMD();
        *task_data = *market_data;
        task->task_data = task_data;
    }

    if (bid1_qty && bid1_count > 0) {
        auto* task_data_one = new int64_t[bid1_count];
        for (int32_t i = 0; i < bid1_count; i++) {
            task_data_one[i] = bid1_qty[i];
        }
        task->task_data_one = task_data_one;
    }
    task->task_one_counts = bid1_count;
    task->task_one_all_counts = max_bid1_count;

    if (ask1_qty && ask1_count > 0) {
        auto* task_data_two = new int64_t[ask1_count];
        for (int32_t i = 0; i < ask1_count; i++) {
            task_data_two[i] = ask1_qty[i];
        }
        task->task_data_two = task_data_two;
    }
    task->task_two_counts = ask1_count;
    task->task_two_all_counts = max_ask1_count;

    task_queue_.push(task);
    return;
}


void QuoteApi::OnSubscribeAllMarketData(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info)
{
    auto *task = new Task();
    task->task_name = ON_SUBSCRIBE_ALL_MARKET_DATA;

    if (error_info) {
        auto* task_error = new XTPRI();
        *task_error = *error_info;
        task->task_error = task_error;
    }

    task->exchange_id = exchange_id;
    task_queue_.push(task);
}

void QuoteApi::OnSubMarketData(XTPST *ticker, XTPRI *error_info, bool is_last)
{
    auto* task = new Task();
    task->task_name = ON_SUB_MARKET_DATA;

    if (ticker) {
        auto* task_data = new XTPST();
        *task_data = *ticker;
        task->task_data = task_data;
    }

    if (error_info) {
        auto* task_error = new XTPRI();
        *task_error = *error_info;
        task->task_error = task_error;
    }

    task->task_last = is_last;
    task_queue_.push(task);
}

void QuoteApi::OnQueryTickersPriceInfo(XTPTPI* ticker_info, XTPRI *error_info, bool is_last)
{
    auto* task = new Task();
    task->task_name = ON_QUERY_TICKERS_PRICE_INFO;

    if (ticker_info) {
        auto* task_data = new XTPTPI();
        *task_data = *ticker_info;
        task->task_data = task_data;
    }

    if (error_info) {
        auto* task_error = new XTPRI();
        *task_error = *error_info;
        task->task_error = task_error;
    }

    task->task_last = is_last;
    task_queue_.push(task);
}

void QuoteApi::OnQueryAllTickersFullInfo(XTPQFI* ticker_info, XTPRI *error_info, bool is_last) 
{
    auto* task = new Task();
    task->task_name = ON_QUERY_ALL_TICKERS_FULL_INFO;

    if (ticker_info) {
        auto* task_data = new XTPQFI();
        *task_data = *ticker_info;
        task->task_data = task_data;
    }

    if (error_info) {
        auto* task_error = new XTPRI();
        *task_error = *error_info;
        task->task_error = task_error;
    }

    task->task_last = is_last;
    task_queue_.push(task);
}

int QuoteApi::login(std::string ip, int port, std::string user, std::string passwd, int socktype, std::string local_ip)
{
    return api_ptr_->Login(ip.c_str(), port, user.c_str(), passwd.c_str(), (XTP_PROTOCOL_TYPE) socktype, local_ip.c_str());
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
    char** tickers = new char*[list_len];
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
    char** tickers = new char*[list_len];
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
    char** tickers = new char*[list_len];
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
    return api_ptr_->QueryAllTickersPriceInfo(); }

void QuoteApi::process_task()
{
    while (true) {
        auto* task = task_queue_.wait_and_pop();
        switch (task->task_name) {
            case ON_DISCONNECTED: 
                process_disconnected(task);
                break;
            case ON_ERROR:
                process_error(task);
                break;
            case ON_SUB_MARKET_DATA:
                process_sub_market_data(task);
                break;
            case ON_UNSUB_MARKET_DATA:
                process_unsub_market_data(task);
                break;
            case ON_DEPTH_MARKET_DATA:
                process_depth_market_data(task);
                break;
            case ON_SUB_ORDER_BOOK:
                process_sub_order_book(task);
                break;
            case ON_UNSUB_ORDER_BOOK:
                process_unsub_order_book(task);
                break;
            case ON_ORDER_BOOK:
                process_order_book(task);
                break;
            case ON_SUB_TICK_BY_TICK:
                process_sub_tick_by_tick(task);
                break;
            case ON_UNSUB_TICK_BY_TICK:
                process_unsub_tick_by_tick(task);
                break;
            case ON_TICK_BY_TICK:
                process_tick_by_tick(task);
                break;
            case ON_SUBSCRIBE_ALL_MARKET_DATA:
                process_subscribe_all_market_data(task);
                break;
            case ON_UNSUBSCRIBE_ALL_MARKET_DATA:
                process_unsubscribe_all_market_data(task);
                break;
            case ON_SUBSCRIBE_ALL_ORDER_BOOK:
                process_subscribe_all_order_book(task);
                break;
            case ON_UNSUBSCRIBE_ALL_ORDER_BOOK:
                process_unsubscribe_all_order_book(task);
                break;
            case ON_SUBSCRIBE_ALL_TICK_BY_TICK:
                process_subscribe_all_tick_by_tick(task);
                break;
            case ON_UNSUBSCRIBE_ALL_TICK_BY_TICK:
                process_unsubscribe_all_tick_by_tick(task);
                break;
            case ON_QUERY_ALL_TICKERS:
                process_query_all_tickers(task);
                break;
            case ON_QUERY_TICKERS_PRICE_INFO:
                process_query_tickers_price_info(task);
                break;
            case ON_SUBSCRIBE_ALL_OPTION_MARKET_DATA:
                process_subscribe_all_option_market_data(task);
                break;
            case ON_UNSUBSCRIBE_ALL_OPTION_MARKET_DATA:
                process_unsubscribe_all_option_market_data(task);
                break;
            case ON_SUBSCRIBE_ALL_OPTION_ORDER_BOOK:
                process_subscribe_all_option_order_book(task);
                break;
            case ON_UNSUBSCRIBE_ALL_OPTION_ORDER_BOOK:
                process_unsubscribe_all_option_order_book(task);
                break;
            case ON_SUBSCRIBE_ALL_OPTION_TICK_BY_TICK:
                process_subscribe_all_option_tick_by_tick(task);
                break;
            case ON_UNSUBSCRIBE_ALL_OPTION_TICK_BY_TICK:
                process_unsubscribe_all_option_tick_by_tick(task);
                break;
            case ON_QUERY_ALL_TICKERS_FULL_INFO:
                process_query_all_tickers_full_info(task);
                break;
        }
    }
}

void QuoteApi::process_disconnected(Task *task)
{
    PyLock lock;
    on_disconnected(task->task_id);
    delete task;
}

void QuoteApi::process_error(Task *task)
{
    PyLock lock;
    py::dict error;
    if (task->task_error) {
        auto* task_error = reinterpret_cast<XTPRI*>(task->task_error);
        error["error_id"] = task_error->error_id;
        error["error_msg"] = task_error->error_msg;
        delete task->task_error;
    }
    on_error(error);
    delete task;
}

void QuoteApi::process_sub_market_data(Task *task)
{
    PyLock lock;
    py::dict data;
    if (task->task_data) {
        auto* task_data = reinterpret_cast<XTPST*>(task->task_data);
        data["exchange_id"] = static_cast<int>(task_data->exchange_id);
        data["ticker"] = add_ending_char(task_data->ticker);
        delete task->task_data;
    }
    py::dict error;
    if (task->task_error) {
        auto* task_error = reinterpret_cast<XTPRI*>(task->task_error);
        error["error_id"] = task_error->error_id;
        error["error_msg"] = add_ending_char(task_error->error_msg);
        delete task->task_error;
    }
    on_sub_market_data(data, error, task->task_last);
    delete task;
}

void QuoteApi::process_unsub_market_data(Task *task)
{
    PyLock lock;
    py::dict data;
    if (task->task_data) {
        auto* task_data = reinterpret_cast<XTPST*>(task->task_data);
        data["exchange_id"] = static_cast<int>(task_data->exchange_id);
        data["ticker"] = add_ending_char(task_data->ticker);
        delete task->task_data;
    }

    py::dict error;
    if (task->task_error) {
        auto* task_error = reinterpret_cast<XTPRI*>(task->task_error);
        error["error_id"] = task_error->error_id;
        error["error_msg"] = add_ending_char(task_error->error_msg);
        delete task->task_error;
    }
    on_unsub_market_data(data, error, task->task_last);
    delete task;
}

void QuoteApi::process_depth_market_data(Task *task)
{
    PyLock lock;
    py::dict data;
    if (task->task_data) {
        auto* task_data = reinterpret_cast<XTPMD*>(task->task_data);
        data["exchange_id"] = static_cast<int>(task_data->exchange_id);
        data["ticker"] = add_ending_char(task_data->ticker);
        data["last_price"] = task_data->last_price;
        data["pre_close_price"] = task_data->pre_close_price;
        data["open_price"] = task_data->open_price;
        data["high_price"] = task_data->high_price;
        data["low_price"] = task_data->low_price;
        data["close_price"] = task_data->close_price;

        data["pct_chg"] = (task_data->last_price - task_data->upper_limit_price) / task_data->upper_limit_price * 100;
        data["pre_total_long_position"] = task_data->pre_total_long_positon;
        data["total_long_position"] = task_data->total_long_positon;
        data["pre_settl_price"] = task_data->pre_settl_price;

        data["upper_limit_price"] = task_data->upper_limit_price;
        data["lower_limit_price"] = task_data->lower_limit_price;
        data["pre_delta"] = task_data->pre_delta;
        data["curr_delta"] = task_data->curr_delta;

        data["data_time"] = task_data->data_time;

        data["qty"] = task_data->qty;
        data["turnover"] = task_data->turnover;
        data["avg_price"] = task_data->avg_price;

        data["trades_count"] = task_data->trades_count;
        char str_ticker_status[9] = {'\0'};
        std::strncpy(str_ticker_status, task_data->ticker_status, sizeof(task_data->ticker_status));
        data["ticker_status"] = add_ending_char(str_ticker_status);

        py::list ask, bid, ask_qty, bid_qty;
        for (int i = 0; i < 10; i++) {
            ask.append(task_data->ask[i]);
            bid.append(task_data->bid[i]);
            ask_qty.append(task_data->ask_qty[i]);
            bid_qty.append(task_data->bid_qty[i]);
        }

        data["ask"] = ask;
        data["bid"] = bid;
        data["bid_qty"] = bid_qty;
        data["ask_qty"] = ask_qty;

        data["data_type"] = static_cast<int>(task_data->data_type);
        if (task_data->data_type == XTP_MARKETDATA_ACTUAL) {
            data["total_bid_qty"] = task_data->stk.total_bid_qty;
            data["total_ask_qty"] = task_data->stk.total_ask_qty;
            data["ma_bid_price"] = task_data->stk.ma_bid_price;
            data["ma_ask_price"] = task_data->stk.ma_ask_price;
            data["ma_bond_bid_price"] = task_data->stk.ma_bond_bid_price;
            data["ma_bond_ask_price"] = task_data->stk.ma_bond_ask_price;
            data["yield_to_maturity"] = task_data->stk.yield_to_maturity;
            data["iopv"] = task_data->stk.iopv;
            data["etf_buy_count"] = task_data->stk.etf_buy_count;
            data["etf_sell_count"] = task_data->stk.etf_sell_count;
            data["etf_buy_qty"] = task_data->stk.etf_buy_qty;
            data["etf_buy_money"] = task_data->stk.etf_buy_money;
            data["etf_sell_qty"] = task_data->stk.etf_sell_qty;
            data["etf_sell_money"] = task_data->stk.etf_sell_money;
            data["total_warrant_exec_qty"] = task_data->stk.total_warrant_exec_qty;
            data["warrant_lower_price"] = task_data->stk.warrant_lower_price;
            data["warrant_upper_price"] = task_data->stk.warrant_upper_price;
            data["cancel_buy_count"] = task_data->stk.cancel_buy_count;
            data["cancel_sell_count"] = task_data->stk.cancel_sell_count;
            data["cancel_buy_qty"] = task_data->stk.cancel_buy_qty;
            data["cancel_sell_qty"] = task_data->stk.cancel_sell_qty;
            data["cancel_buy_money"] = task_data->stk.cancel_buy_money;
            data["cancel_sell_money"] = task_data->stk.cancel_sell_money;
            data["total_buy_count"] = task_data->stk.total_buy_count;
            data["total_sell_count"] = task_data->stk.total_sell_count;
            data["duration_after_buy"] = task_data->stk.duration_after_buy;
            data["duration_after_sell"] = task_data->stk.duration_after_sell;
            data["num_bid_orders"] = task_data->stk.num_bid_orders;
            data["num_ask_orders"] = task_data->stk.num_ask_orders;
            data["pre_iopv"] = task_data->stk.pre_iopv;
            data["r1"] = task_data->stk.r1;
            data["r2"] = task_data->stk.r2;
        } else if (task_data->data_type == XTP_MARKETDATA_OPTION) {
            data["auction_price"] = task_data->opt.auction_price;
            data["auction_qty"] = task_data->opt.auction_qty;
            data["last_enquiry_time"] = task_data->opt.last_enquiry_time;
        }
        data["r4"] = task_data->r4;

        delete task->task_data;
    }
    py::list bid1_qty_list;
    if (task->task_data_one && task->task_one_counts > 0) {
        for (int i = 0; i < task->task_one_counts; i++) {
            auto* bid1_qty = reinterpret_cast<int64_t*>(reinterpret_cast<char*>(task->task_data_one) + i);
            bid1_qty_list.append(*bid1_qty);
        }
        delete[] task->task_data_one;
    }

    int bid1_count = task->task_one_counts;
    int max_bid1_count = task->task_one_all_counts;

    py::list ask1_qty_list;
    if (task->task_data_two && task->task_two_counts > 0) {
        for (int i = 0; i < task->task_two_counts; i++) {
            auto* ask1_qty = reinterpret_cast<int64_t*>(reinterpret_cast<char*>(task->task_data_two) + i);
            ask1_qty_list.append(*ask1_qty);
        }
        delete[] task->task_data_two;
    }

    int ask1_count = task->task_two_counts;
    int max_ask1_count = task->task_two_all_counts;
    on_depth_market_data(data, bid1_qty_list, bid1_count, max_bid1_count, ask1_qty_list, ask1_count, max_ask1_count);
    delete task;
}

void QuoteApi::process_sub_order_book(Task *task)
{
    PyLock lock;
    py::dict data;

    if (task->task_data) {
        auto* task_data = reinterpret_cast<XTPST*>(task->task_data);
        data["exchange_id"] = static_cast<int>(task_data->exchange_id);
        data["ticker"] = add_ending_char(task_data->ticker);
        delete task->task_data;
    }

    py::dict error;
    if (task->task_error) {
        auto* task_error = reinterpret_cast<XTPRI*>(task->task_error);
        error["error_id"] = task_error->error_id;
        error["error_msg"]= task_error->error_msg;
        delete task->task_error;
    }

    on_sub_order_book(data, error, task->task_last);
    delete task;
}

void QuoteApi::process_unsub_order_book(Task *task)
{
    PyLock lock;
    py::dict data;

    if (task->task_data) {
        auto* task_data = reinterpret_cast<XTPST*>(task->task_data);
        data["exchange_id"] = static_cast<int>(task_data->exchange_id);
        data["ticker"] = add_ending_char(task_data->ticker);
        delete task->task_data;
    }

    py::dict error;
    if (task->task_error) {
        auto* task_error = reinterpret_cast<XTPRI*>(task->task_error);
        error["error_id"] = task_error->error_id;
        error["error_msg"]= task_error->error_msg;
        delete task->task_error;
    }

    on_unsub_order_book(data, error, task->task_last);
    delete task;
}

void QuoteApi::process_order_book(Task *task)
{
    PyLock lock;
    py::dict data;
    if (task->task_data) {
        auto* task_data = reinterpret_cast<XTPOB*>(task->task_data);
        data["exchange_id"] = static_cast<int>(task_data->exchange_id);
        data["ticker"] = add_ending_char(task_data->ticker);
        data["last_price"] = task_data->last_price;
        data["qty"] = task_data->qty;
        data["turnover"] = task_data->turnover;
        data["trades_count"] = task_data->trades_count;

        py::list ask, bid, ask_qty, bid_qty;
        for (int i = 0; i < 10; i++) {
            ask.append(task_data->ask[i]);
            bid.append(task_data->bid[i]);
            ask_qty.append(task_data->ask_qty[i]);
            bid_qty.append(task_data->bid_qty[i]);
        }

        data["ask"] = ask;
        data["bid"] = bid;
        data["ask_qty"] = ask_qty;
        data["bid_qty"] = bid_qty;
        
        delete task->task_data;
    }

    on_order_book(data);
    delete task;
}

void QuoteApi::process_sub_tick_by_tick(Task *task)
{
    PyLock lock;
    py::dict data;
    if (task->task_data) {
        auto* task_data = reinterpret_cast<XTPST*>(task->task_data);
        data["exchange_id"] = static_cast<int>(task_data->exchange_id);
        data["ticker"] = add_ending_char(task_data->ticker);
        delete task->task_data;
    }

    py::dict error;
    if (task->task_error) {
        auto* task_error = reinterpret_cast<XTPRI*>(task->task_error);
        error["error_id"] = task_error->error_id;
        error["error_msg"] = task_error->error_msg;
        delete task->task_error;
    }

    on_sub_tick_by_tick(data, error, task->task_last);
    delete task;
}

void QuoteApi::process_unsub_tick_by_tick(Task *task)
{
    PyLock lock;
    py::dict data;
    if (task->task_data) {
        auto* task_data = reinterpret_cast<XTPST*>(task->task_data);
        data["exchange_id"] = static_cast<int>(task_data->exchange_id);
        data["ticker"] = add_ending_char(task_data->ticker);
        delete task->task_data;
    }

    py::dict error;
    if (task->task_error) {
        auto* task_error = reinterpret_cast<XTPRI*>(task->task_error);
        error["error_id"] = task_error->error_id;
        error["error_msg"] = task_error->error_msg;
        delete task->task_error;
    }

    on_unsub_tick_by_tick(data, error, task->task_last);
    delete task;
}

void QuoteApi::process_tick_by_tick(Task *task)
{
    PyLock lock;
    py::dict data;
    if (task->task_data) {
        auto* task_data = reinterpret_cast<XTPTBT*>(task->task_data);
        data["exchange_id"] = static_cast<int>(task_data->exchange_id);
        data["ticker"] = add_ending_char(task_data->ticker);
        data["data_time"] = task_data->data_time;
        data["type"] = static_cast<int>(task_data->type);
        if (task_data->type == XTP_TBT_ENTRUST) {
            const auto& order = task_data->entrust;
            data["channel_no"] = order.channel_no;
            data["seq"] = order.seq;
            data["price"] = order.price;
            data["qty"] = order.qty;
            data["side"] = order.side;
            data["ord_type"] = order.ord_type;
            data["order_no"] = order.order_no;
        } else {
            const auto& trade = task_data->trade;
            data["channel_no"] = trade.channel_no;
            data["seq"] = trade.seq;
            data["price"] = trade.price;
            data["qty"] = trade.qty;
            data["money"] = trade.money;
            data["bid_no"] = trade.bid_no;
            data["ask_no"] = trade.ask_no;
            data["trade_flag"] = trade.trade_flag;
        }

        delete task->task_data;
    }

    on_tick_by_tick(data);
    delete task;
}

void QuoteApi::process_subscribe_all_market_data(Task *task)
{
    PyLock lock;
    py::dict error;
    if (task->task_error) {
        auto* task_error = reinterpret_cast<XTPRI*>(task->task_error);
        error["error_id"] = task_error->error_id;
        error["error_msg"] = task_error->error_msg;
        delete task->task_error;
    }

    on_subscribe_all_market_data(task->exchange_id, error);
    delete task;
}

void QuoteApi::process_unsubscribe_all_market_data(Task *task)
{
}

void QuoteApi::process_subscribe_all_order_book(Task *task)
{}

void QuoteApi::process_unsubscribe_all_order_book(Task *task)
{

}

void QuoteApi::process_subscribe_all_tick_by_tick(Task *task)
{
    PyLock lock;

    py::dict error;
	if (task->task_error) {
		auto* task_error = reinterpret_cast<XTPRI*>(task->task_error);
		error["error_id"] = task_error->error_id;
		error["error_msg"] = add_ending_char(task_error->error_msg);
		delete task->task_error;
	}

	on_subscribe_all_tick_by_tick(task->exchange_id, error);
	delete task;
}

void QuoteApi::process_unsubscribe_all_tick_by_tick(Task *task)
{
    PyLock lock;

    py::dict error;
	if (task->task_error) {
		auto* task_error = reinterpret_cast<XTPRI*>(task->task_error);
		error["error_id"] = task_error->error_id;
		error["error_msg"] = add_ending_char(task_error->error_msg);
		delete task->task_error;
	}

	on_unsubscribe_all_tick_by_tick(task->exchange_id, error);
	delete task;
}

void QuoteApi::process_query_all_tickers(Task *task)
{
    PyLock lock;

    py::dict data;
    if (task->task_data) {
        auto* task_data = reinterpret_cast<XTPQSI*>(task->task_data);
        data["exchange_id"] = static_cast<int>(task_data->exchange_id);
        data["ticker"] = add_ending_char(task_data->ticker);
        data["ticker_name"] = add_ending_char(task_data->ticker_name);
        data["ticker_type"] = static_cast<int>(task_data->ticker_type);
        data["pre_close_price"] = task_data->pre_close_price;
        data["upper_limit_price"] = task_data->upper_limit_price;
        data["lower_limit_price"] = task_data->lower_limit_price;
        data["price_tick"] = task_data->price_tick;
        data["buy_qty_unit"] = task_data->buy_qty_unit;
        data["sell_qty_unit"] = task_data->sell_qty_unit;

        delete task->task_data;
    }

    py::dict error;
    if (task->task_error)
    {
        auto* task_error = reinterpret_cast<XTPRI*>(task->task_error);
        error["error_id"] = task_error->error_id;
        error["error_msg"] = add_ending_char(task_error->error_msg);
        delete task->task_error;
    }

    on_query_all_tickers(data, error, task->task_last);
    delete task;
}

void QuoteApi::process_query_tickers_price_info(Task *task)
{
    PyLock lock;
    py::dict data;

    if (task->task_data) {
        auto* task_data = reinterpret_cast<XTPTPI*>(task->task_data);
        data["exchange_id"] = static_cast<int>(task_data->exchange_id);
		data["ticker"] = add_ending_char(task_data->ticker);
		data["last_price"] = task_data->last_price;

        delete task->task_data;
    }

    py::dict error;
    if (task->task_error) {
        auto* task_error = reinterpret_cast<XTPRI*>(task->task_error);
        error["error_id"] = task_error->error_id;
		error["error_msg"] = add_ending_char(task_error->error_msg);
		delete task->task_error;
    }

    on_query_tickers_price_info(data, error, task->task_last);
    delete task;
}

void QuoteApi::process_subscribe_all_option_market_data(Task *task)
{}

void QuoteApi::process_unsubscribe_all_option_market_data(Task *task)
{}

void QuoteApi::process_subscribe_all_option_order_book(Task *task)
{}

void QuoteApi::process_unsubscribe_all_option_order_book(Task *task)
{}

void QuoteApi::process_subscribe_all_option_tick_by_tick(Task *task)
{}

void QuoteApi::process_unsubscribe_all_option_tick_by_tick(Task *task)
{}

void QuoteApi::process_query_all_tickers_full_info(Task *task)
{
    PyLock lock;
    py::dict data;

    if (task->task_data) {
        auto* task_data = reinterpret_cast<XTPQFI*>(task->task_data);
        data["exchange_id"] = static_cast<int>(task_data->exchange_id);
		data["ticker"] = add_ending_char(task_data->ticker);
		data["ticker_name"] = add_ending_char(task_data->ticker_name);
		data["security_type"] = static_cast<int>(task_data->security_type);
		data["ticker_qualification_class"] = (int)task_data->ticker_qualification_class;
		data["is_registration"] = task_data->is_registration;
		data["is_VIE"] = task_data->is_VIE;
		data["is_noprofit"] = task_data->is_noprofit;
		data["is_weighted_voting_rights"] = task_data->is_weighted_voting_rights;
		data["is_have_price_limit"] = task_data->is_have_price_limit;
		data["upper_limit_price"] = task_data->upper_limit_price;
		data["lower_limit_price"] = task_data->lower_limit_price;
		data["pre_close_price"] = task_data->pre_close_price;
		data["price_tick"] = task_data->price_tick;
		data["bid_qty_upper_limit"] = task_data->bid_qty_upper_limit;
		data["bid_qty_lower_limit"] = task_data->bid_qty_lower_limit;
		data["bid_qty_unit"] = task_data->bid_qty_unit;
		data["ask_qty_upper_limit"] = task_data->ask_qty_upper_limit;
		data["ask_qty_lower_limit"] = task_data->ask_qty_lower_limit;
		data["ask_qty_unit"] = task_data->ask_qty_unit;
		data["market_bid_qty_upper_limit"] = task_data->market_bid_qty_upper_limit;
		data["market_bid_qty_lower_limit"] = task_data->market_bid_qty_lower_limit;
		data["market_bid_qty_unit"] = task_data->market_bid_qty_unit;
		data["market_ask_qty_upper_limit"] = task_data->market_ask_qty_upper_limit;
		data["market_ask_qty_lower_limit"] = task_data->market_ask_qty_lower_limit;
		data["market_ask_qty_unit"] = task_data->market_ask_qty_unit;
		data["security_status"] = static_cast<int>(task_data->security_status);

        delete task->task_data;
    }

    py::dict error;
    if (task->task_error) {
        auto* task_error = reinterpret_cast<XTPRI*>(task->task_error);
        error["error_id"] = task_error->error_id;
        error["error_msg"] = task_error->error_msg;
        delete task->task_error;
    }

    on_query_all_tickers_full_info(data, error, task->task_last);
    delete task;
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
    return (p == nullptr) ? "NULL": p;
}

std::string QuoteApi::get_api_version()
{
    auto p = api_ptr_->GetApiVersion();
    return (p == nullptr) ? "NULL": p;
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

struct QuoteApiWrap: QuoteApi
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

    void on_depth_market_data(py::dict data, py::list bid1_qty_list, int bid1_count, int max_bid1_count, py::list ask1_qty_list, int ask1_count, int max_ask1_count) override
    {
        try {
            pybind11::get_overload(this, "on_depth_market_data")(data, bid1_qty_list, bid1_count, max_bid1_count, ask1_qty_list, ask1_count, max_ask1_count);
            // printf("on_depth_market_data===>bid1_count:%d, ask1_count:%d\n", bid1_count, ask1_count);
        } catch (pybind11::error_already_set const &) {
            PyErr_Print();
        }
    }

    void on_query_all_tickers_full_info(py::dict data, py::dict error, bool last) override
    {
        PyLock lock;
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
    PyEval_InitThreads();

    m.doc() = "XTP SDK";

    py::class_<QuoteApiWrap>(m, "QuoteApi")
        .def(py::init<>())
        .def("get_api_version", &QuoteApiWrap::get_api_version, "获取API的发行版本号")
        .def("get_trading_day", &QuoteApiWrap::get_trading_day, "获取当前交易日")
        .def("get_api_last_error", &QuoteApiWrap::get_api_last_error, "获取API的系统错误,可以在Login、Logout、订阅、取消订阅失败时调用,获取失败的原因")
        .def("create_quote_api", &QuoteApiWrap::create_quote_api, "创建QuoteApi,返回创建出的UserApi",
                py::arg("client_id"), py::arg("log_path"), py::arg("log_level"))
        .def("query_all_tickers_full_info", &QuoteApiWrap::query_all_tickers_full_info, "获取所有合约的详细静态信息,包括指数等非可交易的,“0”表示发送查询请求成功,非“0”表示发送查询请求不成功", py::arg("exchange_id"))
        .def("login", &QuoteApiWrap::login, "用户登录请求,“0”表示登录成功,“-1”表示连接服务器出错,此时用户可以调用GetApiLastError()来获取错误代码,“-2”表示已存在连接,不允许重复登录,如果需要重连,请先logout,“-3”表示输入有错误",
                py::arg("ip"), py::arg("port"), py::arg("user"), py::arg("passwd"), py::arg("socktype"), py::arg("local_ip"))
        .def("logout", &QuoteApiWrap::logout, "登出请求,“0”表示登出成功,非“0”表示登出出错,此时用户可以调用GetApiLastError()来获取错误代码")
        .def("query_all_tickers_price_info", &QuoteApiWrap::query_all_tickers_price_info, "获取所有合约的最新价格信息,“0”表示发送查询请求成功,非“0”表示发送查询请求不成功")
        .def("query_tickers_price_info", &QuoteApiWrap::query_tickers_price_info, "获取合约的最新价格信息,“0”表示发送查询请求成功,非“0”表示发送查询请求不成功", py::arg("ticker"), py::arg("count"), py::arg("exchange_id"))
        .def("subscribe_market_data", &QuoteApiWrap::subscribe_market_data, "订阅行情,包括股票、指数和期权,“0”表示接口调用成功,非“0”表示接口调用出错", py::arg("ticker"), py::arg("count"), py::arg("exchange_id"))
        .def("unsubscribe_market_data", &QuoteApiWrap::unsubscribe_market_data, "退订行情,包括股票、指数和期权, “0”表示接口调用成功,非“0”表示接口调用出错", py::arg("ticker"), py::arg("count"), py::arg("exchange_id"))
        .def("subscribe_all_market_data", &QuoteApiWrap::subscribe_all_market_data, "订阅全市场的股票行情", py::arg("exchange_id"))
        .def("unsubscribe_all_market_data", &QuoteApiWrap::unsubscribe_all_market_data, "退订全市场的股票行情", py::arg("exchange_id"))
        .def("subscribe_all_tick_by_tick", &QuoteApiWrap::subscribe_all_tick_by_tick, "订阅全市场的股票逐笔行情,“0”表示接口调用成功,非“0”表示接口调用出错", py::arg("exchange_id"))
        .def("unsubscribe_all_tick_by_tick", &QuoteApiWrap::unsubscribe_all_tick_by_tick, "退订全市场的股票逐笔行情,“0”表示接口调用成功,非“0”表示接口调用出错", py::arg("exchange_id"))
        .def("on_depth_market_data", &QuoteApiWrap::on_depth_market_data, "深度行情通知,包含买一卖一队列", py::arg("market_data"), py::arg("bid1_qty"), py::arg("bid1_count"),
                py::arg("max_bid1_count"), py::arg("ask1_qty"), py::arg("ask1_count"), py::arg("max_ask1_count"))
        .def("on_disconnected", &QuoteApiWrap::on_disconnected, "当客户端与行情后台通信连接断开时,该方法被调用",
                py::arg("reason"))
        .def("on_error", &QuoteApiWrap::on_error, "错误应答", py::arg("error_info"))
        .def("on_sub_market_data", &QuoteApiWrap::on_sub_market_data, "订阅行情应答,包括股票、指数和期权",
                py::arg("ticker"), py::arg("error_info"), py::arg("is_last"))
        .def("on_query_all_tickers", &QuoteApiWrap::on_query_all_tickers, "查询合约部分静态信息的应答",
                py::arg("ticker_info"), py::arg("error_info"), py::arg("is_last"))
        .def("on_query_tickers_price_info", &QuoteApiWrap::on_query_tickers_price_info, "查询合约的最新价格信息应答",
                py::arg("ticker_info"), py::arg("error_info"), py::arg("is_last"))
        .def("on_query_all_tickers_full_info", &QuoteApiWrap::on_query_all_tickers_full_info, "查询合约完整静态信息的应答", 
                py::arg("ticker_info"), py::arg("error_info"), py::arg("is_last"))
        .def("on_subscribe_all_market_data", &QuoteApi::on_subscribe_all_market_data, "订阅全市场的股票行情应答",
                py::arg("exchange_id"), py::arg("error_info"))
        ;
}

