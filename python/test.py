import os
import sys
import json

from datetime import datetime
from api import QuoteApi
from api.xtp_types import XTP_EXCHANGE_TYPE, XTP_PROTOCOL_TYPE, XTP_LOG_LEVEL


config = json.load(open("config.json", encoding="utf-8"))
xtp_config = config["xtp"]
USER = xtp_config["user"]
PASS = xtp_config["pass"]
HOST = xtp_config["host"]
PORT = xtp_config["port"]
PROTOCOL_TYPE = xtp_config["socket_type"]
CLIENT_ID = xtp_config["client_id"]


class Md(QuoteApi):

    def __init__(self):
        super().__init__()
        self.data = {}

    def on_disconnected(self, reason: int):
        """
        当客户端与行情后台通信连接断开时,该方法被调用
        :param reason: 错误原因,请与错误代码表对应
        """
        print("disconnected")
        sys.exit(1)

    def on_query_all_tickers_full_info(self, ticker_info: dict, error_info: dict, is_last: bool):
        """
        查询合约完整静态信息的应答

        :param ticker_info: 合约完整静态信息
        :param error_info: 查询合约完整静态信息时发生错误时返回的错误信息,当error_info为空,或者error_info.error_id为0时,表明没有错误
        :param is_last: 是否此次查询合约完整静态信息的最后一个应答,当为最后一个的时候为true,如果为false,表示还有其他后续消息响应
        """
        if ticker_info:
            print("on_query_all_tickers_full_info:", ticker_info)
        else:
            print("on_query_all_tickers_full_info:", error_info)

    def on_query_all_tickers(self, ticker_info: dict, error_info: dict, is_last: bool):
        """
        查询合约部分静态信息的应答

        :param ticker_info: 合约部分静态信息
        :param error_info: 查询合约部分静态信息时发生错误时返回的错误信息,当error_info为空,或者error_info.error_id为0时,表明没有错误
        :param is_last: 是否此次查询合约部分静态信息的最后一个应答,当为最后一个的时候为true,如果为false,表示还有其他后续消息响应
        """
        print(ticker_info)

    def on_query_tickers_price_info(self, ticker_info: dict, error_info: dict, is_last: bool):
        """
        查询合约的最新价格信息应答

        :param ticker_info: 合约的最新价格信息
        :param error_info: 查询合约部分静态信息时发生错误时返回的错误信息,当error_info为空,或者error_info.error_id为0时,表明没有错误
        :param is_last: 是否此次查询合约部分静态信息的最后一个应答,当为最后一个的时候为true,如果为false,表示还有其他后续消息响应
        """
        print(ticker_info)

    def on_depth_market_data(self, market_data: dict, bid1_qty: list, bid1_count: int, max_bid1_count: int, 
            ask1_qty: list, ask1_count: int, max_ask1_count: int):
        """
        深度行情通知,包含买一卖一队列
        """
        print(market_data)

    def on_subscribe_all_market_data(self, exchange_id: int, error: dict):
        """
        订阅全市场的股票行情应答

        :param exchange_id: 表示当前全订阅的市场,如果为3,表示沪深全市场,1 表示为上海全市场,2 表示为深圳全市场
        :param error: 取消订阅合约时发生错误时返回的错误信息,当error_info为空,或者error_info.error_id为0时,表明没有错误
        """
        if error["error_id"] == 0:
            print(exchange_id)
        else:
            print(error)

    def on_sub_market_data(self, ticker: dict, error_info: dict, is_last: bool):
        """
        订阅行情应答,包括股票、指数和期权
        *每条订阅的合约均对应一条订阅应答,需要快速返回,否则会堵塞后续消息,当堵塞严重时,会触发断线

        :param ticker: 详细的合约订阅情况
        :param error_info: 订阅合约发生错误时的错误信息,当error_info为空,或者error_info.error_id为0时,表明没有错误
        :param is_last: 是否此次订阅的最后一个应答,当为最后一个的时候为true,如果为false,表示还有其他后续消息响应
        """
        print("on_sub_market_data:", ticker, " - ", error_info)
        self.data[ticker["ticker"]] = ticker


def main():
    md = Md()
    md.create_quote_api(CLIENT_ID, os.getcwd(), XTP_LOG_LEVEL.XTP_LOG_LEVEL_DEBUG)
    if md.login(HOST, PORT, USER, PASS, PROTOCOL_TYPE, "0") != 0:
        print("Login failed!")
        md.get_api_last_error()
        sys.exit(1)

    print("TradingDay:", md.get_trading_day())
    print("Login success")

    s = md.subscribe_all_market_data(XTP_EXCHANGE_TYPE.XTP_EXCHANGE_UNKNOWN)
    if s != 0:
        err = md.get_api_last_error()
        print(err)

    while int(datetime.now().strftime("%H%M%S")) <= 150000:
        pass


if __name__ == "__main__":
    main()

