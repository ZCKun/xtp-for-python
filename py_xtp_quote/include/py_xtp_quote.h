#pragma once

#include "pybind11/pytypes.h"
#include "pybind11/embed.h"
#include "xquote_api_struct.h"
#include "xtp_quote_api.h"

#include <condition_variable>
#include <functional>
#include <mutex>
#include <pybind11/pybind11.h>

#include <queue>
#include <string>
#include <thread>
#include <utility>

namespace py = pybind11;

#define ON_DISCONNECTED 1
#define ON_ERROR 2
#define ON_SUB_MARKET_DATA 3
#define ON_UNSUB_MARKET_DATA 4
#define ON_DEPTH_MARKET_DATA 5
#define ON_SUB_ORDER_BOOK 6
#define ON_UNSUB_ORDER_BOOK 7
#define ON_ORDER_BOOK 8
#define ON_SUB_TICK_BY_TICK 9
#define ON_UNSUB_TICK_BY_TICK 10
#define ON_TICK_BY_TICK 11
#define ON_SUBSCRIBE_ALL_MARKET_DATA 12
#define ON_UNSUBSCRIBE_ALL_MARKET_DATA 13
#define ON_SUBSCRIBE_ALL_ORDER_BOOK 14
#define ON_UNSUBSCRIBE_ALL_ORDER_BOOK 15
#define ON_SUBSCRIBE_ALL_TICK_BY_TICK 16
#define ON_UNSUBSCRIBE_ALL_TICK_BY_TICK 17
#define ON_QUERY_ALL_TICKERS 18
#define ON_QUERY_TICKERS_PRICE_INFO 19
#define ON_SUBSCRIBE_ALL_OPTION_MARKET_DATA 20
#define ON_UNSUBSCRIBE_ALL_OPTION_MARKET_DATA 21
#define ON_SUBSCRIBE_ALL_OPTION_ORDER_BOOK 22
#define ON_UNSUBSCRIBE_ALL_OPTION_ORDER_BOOK 23
#define ON_SUBSCRIBE_ALL_OPTION_TICK_BY_TICK 24
#define ON_UNSUBSCRIBE_ALL_OPTION_TICK_BY_TICK 25
#define ON_QUERY_ALL_TICKERS_FULL_INFO 26

/*!
 * 用于帮助C++线程获得GIL锁，从而防止python崩溃
 */
class PyLock
{
private:
    PyGILState_STATE gil_state;
public:
    /*!
     * 在某个函数方法中创建该对象时，获得GIL锁
     */
    PyLock()
    {
        gil_state = PyGILState_Ensure();
    }

    /*!
     * 在某个函数完成后销毁该对象时，解放GIL锁
     */
    ~PyLock()
    {
        PyGILState_Release(gil_state);
    }
};

void get_int(py::dict d, std::string key, int *value);

void get_double(py::dict d, std::string key, double *value);

void get_char(py::dict d, std::string key, char *value);

void get_str(py::dict &d, const std::string &key, char *value);

struct Task
{
    int task_name;
    int task_id;
    int exchange_id;
    int task_one_counts;
    int task_one_all_counts;
    int task_two_counts;
    int task_two_all_counts;
    void *task_data;
    void *task_error;
    void *task_data_one;
    void *task_data_two;
    bool task_last;
};

template<typename T>
class ConcurrentQueue
{
private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable condition_variable_;

public:
    void push(T const &data)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(data);
        }

        condition_variable_.notify_one();
    }

    bool empty() const noexcept
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    T wait_and_pop()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (queue_.empty()) {
            condition_variable_.wait(lock);
        }
        auto data = queue_.front();
        queue_.pop();
        return data;
    }
};


class QuoteApi : public XTP::API::QuoteSpi
{

private:
    XTP::API::QuoteApi *api_ptr_;
    std::thread *task_thread_ptr_;
    ConcurrentQueue<Task *> task_queue_;

public:
    QuoteApi()
            : api_ptr_(nullptr),
              task_thread_ptr_(nullptr)
    {
//        std::thread([&] {
//            process_task();
//        }).detach();
        // task_thread_ptr_ = &t;
    }

    ~QuoteApi() = default;

    // void OnDisconnected(int reason) override;

    void OnError(XTPRI *error_info) override;

    void OnSubMarketData(XTPST *ticker, XTPRI *error_info, bool is_last) override;

    void OnDepthMarketData(XTPMD *market_data, int64_t bid1_qty[], int32_t bid1_count, int32_t max_bid1_count,
                           int64_t ask1_qty[], int32_t ask1_count, int32_t max_ask1_count) override;

    void OnSubscribeAllMarketData(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) override;

    void OnQueryAllTickersFullInfo(XTPQFI *ticker_info, XTPRI *error_info, bool is_last) override;

    void OnQueryTickersPriceInfo(XTPTPI *ticker_info, XTPRI *error_info, bool is_last) override;

    // -----------------------
    int login(std::string ip, int port, std::string user, std::string passwd, int socktype, std::string local_ip);

    int logout();

    int subscribe_all_tick_by_tick(int exchange = 3);

    int unsubscribe_all_tick_by_tick(int exchange = 3);

    int subscribe_market_data(py::list ticker_list, int count, int exchange);

    int unsubscribe_market_data(py::list ticker_list, int count, int exchange);

    int subscribe_all_market_data(int exchange = 3);

    int unsubscribe_all_market_data(int exchange = 3);

    int query_all_tickers_full_info(int exchange);

    /*!
     * @param ticker_list 需要查询的股票代码列表
     * @param count 查询股票代码数量
     * @param exchange 交易所
     */
    int query_tickers_price_info(py::list ticker_list, int count, int exchange);

    int query_all_tickers_price_info();

    // -----------------------
    virtual void on_disconnected(int reason)
    {};

    virtual void on_error(py::dict data)
    {};

    virtual void on_sub_market_data(py::dict data, py::dict error, bool last)
    {};

    virtual void on_unsub_market_data(py::dict data, py::dict error, bool last)
    {};

    virtual void on_depth_market_data(py::dict data,
                                      py::list bid1_qty_list, int bid1_count, int max_bid1_count,
                                      py::list ask1_qty_list, int ask1_count, int max_ask1_count)
    {};

    virtual void on_sub_order_book(py::dict data, py::dict error, bool last)
    {};

    virtual void on_unsub_order_book(py::dict data, py::dict error, bool last)
    {};

    virtual void on_order_book(py::dict data)
    {}

    virtual void on_sub_tick_by_tick(py::dict data, py::dict error, bool last)
    {};

    virtual void on_unsub_tick_by_tick(py::dict data, py::dict error, bool last)
    {};

    virtual void on_tick_by_tick(py::dict data)
    {};

    virtual void on_subscribe_all_market_data(int exchange_id, py::dict error)
    {};

    virtual void on_unsubscribe_all_market_data(int exchange_id, py::dict error)
    {};

    virtual void on_subscribe_all_order_book(int exchange_id, py::dict error)
    {};

    virtual void on_unsubscribe_all_order_book(int exchange_id, py::dict error)
    {};

    virtual void on_subscribe_all_tick_by_tick(int exchange_id, py::dict error)
    {};

    virtual void on_unsubscribe_all_tick_by_tick(int exchange_id, py::dict error)
    {};

    virtual void on_query_all_tickers(py::dict data, py::dict error, bool last)
    {};

    virtual void on_query_tickers_price_info(py::dict data, py::dict error, bool last)
    {};

    virtual void on_query_all_tickers_full_info(py::dict data, py::dict error, bool last)
    {};

    virtual void on_subscribe_all_option_market_data(int exchange_id, py::dict error)
    {};

    virtual void on_unsubscribe_all_option_market_data(int exchange_id, py::dict error)
    {};

    virtual void on_subscribe_all_option_order_book(int exchange_id, py::dict error)
    {};

    virtual void on_unsubscribe_all_option_order_book(int exchange_id, py::dict error)
    {};

    virtual void on_subscribe_all_option_tick_by_tick(int exchange_id, py::dict error)
    {};

    virtual void on_unsubscribe_all_option_tick_by_tick(int exchange_id, py::dict error)
    {};

    // -----------------------
    //
    int exit();

    void release();

    void create_quote_api(int client_id, std::string path, int log_level);

    std::string get_api_version();

    std::string get_trading_day();

    py::dict get_api_last_error();

    void process_query_all_tickers_full_info(const XTPQFI &data, const XTPRI &error, bool is_last);

    void process_query_tickers_price_info(const XTPTPI &data, const XTPRI &error, bool is_last);

    void process_sub_market_data(const XTPST &data, const XTPRI &error, bool is_last);

    void process_subscribe_all_market_data(XTP_EXCHANGE_TYPE exchange_id, const XTPRI &error);

    void process_depth_market_data(const XTPMD &data, py::list& bid1_qty_queue, int32_t bid1_count, int32_t max_bid1_count,
                                   py::list& ask1_qty_queue, int32_t ask1_count, int32_t max_ask1_count);

    void on_error(const XTPRI &error);
};
