import sys

from . import py_xtp_quote
from .xtp_types import XTP_EXCHANGE_TYPE, XTP_PROTOCOL_TYPE, XTP_LOG_LEVEL


class QuoteApi(py_xtp_quote.QuoteApi):

    def __init__(self):
        super().__init__()

    def on_errr(self, error_info: dict):
        """
        错误应答

        :param error_info: 当服务器响应发生错误时的具体的错误代码和错误信息,当error_info为空,或者error_info.error_id为0时,表明没有错误
        """
        print(error_info)

    def on_disconnected(self, reason: int):
        """
        当客户端与行情后台通信连接断开时,该方法被调用

        :param reason: 错误原因,请与错误代码表对应
        """
        raise NotImplementedError

    def login(self, ip: str, port: int, user: str, passwd: str, sock_type: XTP_PROTOCOL_TYPE, local_ip: str) -> int:
        """
        用户登录请求
        *此函数为同步阻塞式,不需要异步等待登录成功,当函数返回即可进行后续操作,此api只能有一个连接

        :param ip: 服务器ip地址,类似"127.0.0.1"
        :param port: 服务器端口号
        :param user: 登陆用户名
        :param passwd: 登陆密码
        :param sock_type: "1"代表TCP,"2"代表UDP
        :param local_ip: 本地网卡地址,类似"127.0.0.1"
        :return: “0”表示登录成功,“-1”表示连接服务器出错,此时用户可以调用GetApiLastError()来获取错误代码,“-2”表示已存在连接,不允许重复登录,如果需要重连,请先logout,“-3”表示输入有错误
        """
        return super().login(ip, port, user, passwd, sock_type, local_ip)

    def create_quote_api(self, client_id: int, log_path: str, log_level: XTP_LOG_LEVEL):
        """
        创建QuoteApi,返回创建出的UserApi

        :param client_id: (必须输入)用于区分同一用户的不同客户端,由用户自定义
        :param log_path: (必须输入)存贮订阅信息文件的目录,请设定一个有可写权限的真实存在的路径,如果路径不存在的话,可能会因为写冲突而造成断线
        :param log_level:
        """
        super().create_quote_api(client_id, log_path, log_level)

    def get_api_last_error(self):
        """ 获取API的系统错误,可以在Login、Logout、订阅、取消订阅失败时调用,获取失败的原因 """
        return super().get_api_last_error()

    def query_all_tickers_full_info(self, exchange_id: XTP_EXCHANGE_TYPE) -> int:
        """
        获取所有合约的详细静态信息,包括指数等非可交易的

        :param exchange_id: 交易所代码,必须提供 1-上海 2-深圳
        :return: “0”表示发送查询请求成功,非“0”表示发送查询请求不成功
        """
        return super().query_all_tickers_full_info(exchange_id)

    def subscribe_market_data(self, ticker: list, count: int, exchange_id: XTP_EXCHANGE_TYPE) -> int:
        """
        订阅行情,包括股票、指数和期权

        :param ticker: 合约ID数组,注意合约代码不包含空格
        :param count: 要订阅/退订行情的合约个数
        :param exchange_id: 交易所代码
        :return: “0”表示接口调用成功,非“0”表示接口调用出错
        """
        return super().subscribe_market_data(ticker, count, exchange_id)

    def subscribe_all_market_data(self, exchange_id: XTP_EXCHANGE_TYPE = XTP_EXCHANGE_TYPE.XTP_EXCHANGE_UNKNOWN) -> int:
        """
        订阅全市场的股票行情
        *需要与全市场退订行情接口配套使用

        :param exchange_id: 表示当前全订阅的市场,如果为3,表示沪深全市场,1表示为上海全市场,2表示为深圳全市场
        :return: 订阅全市场行情接口调用是否成功,"0"表示接口调用成功,非"0"表示接口调用出错
        """
        return super().subscribe_all_market_data(exchange_id)

    def query_tickers_price_info(self, ticker: list, count: int, exchange_id: XTP_EXCHANGE_TYPE) -> int:
        """
        获取合约的最新价格信息

        :param ticker: 合约ID数组,注意合约代码不包含空格
        :param count: 要查询的合约个数
        :exchange_id: 交易所代码
        :return: “0”表示发送查询请求成功,非“0”表示发送查询请求不成功
        """
        return super().query_tickers_price_info(ticker, count, exchange_id)

    def query_all_tickers_price_info(self) -> int:
        """ 获取所有合约的最新价格信息,“0”表示发送查询请求成功,非“0”表示发送查询请求不成功 """
        return super().query_all_tickers_price_info()

    def on_query_all_tickers_full_info(self, ticker_info: dict, error_info: dict, is_last: bool):
        """
        查询合约完整静态信息的应答

        :param ticker_info: 合约完整静态信息
        :param error_info: 查询合约完整静态信息时发生错误时返回的错误信息,当error_info为空,或者error_info.error_id为0时,表明没有错误
        :param is_last: 是否此次查询合约完整静态信息的最后一个应答,当为最后一个的时候为true,如果为false,表示还有其他后续消息响应
        """
        raise NotImplementedError

    def on_query_all_tickers(self, ticker_info: dict, error_info: dict, is_last: bool):
        """
        查询合约部分静态信息的应答

        :param ticker_info: 合约部分静态信息
        :param error_info: 查询合约部分静态信息时发生错误时返回的错误信息,当error_info为空,或者error_info.error_id为0时,表明没有错误
        :param is_last: 是否此次查询合约部分静态信息的最后一个应答,当为最后一个的时候为true,如果为false,表示还有其他后续消息响应
        """
        raise NotImplementedError

    def on_query_tickers_price_info(self, ticker_info: dict, error_info: dict, is_last: bool):
        """
        查询合约的最新价格信息应答

        :param ticker_info: 合约的最新价格信息
        :param error_info: 查询合约部分静态信息时发生错误时返回的错误信息,当error_info为空,或者error_info.error_id为0时,表明没有错误
        :param is_last: 是否此次查询合约部分静态信息的最后一个应答,当为最后一个的时候为true,如果为false,表示还有其他后续消息响应
        """
        raise NotImplementedError

    def on_depth_market_data(self, market_data: dict, bid1_qty: list, bid1_count: int, max_bid1_count: int, 
            ask1_qty: list, ask1_count: int, max_ask1_count: int):
        """
        深度行情通知,包含买一卖一队列
        """
        raise NotImplementedError

    def on_subscribe_all_market_data(self, exchange_id: int, error: dict):
        """
        订阅全市场的股票行情应答

        :param exchange_id: 表示当前全订阅的市场,如果为3,表示沪深全市场,1 表示为上海全市场,2 表示为深圳全市场
        :param error: 取消订阅合约时发生错误时返回的错误信息,当error_info为空,或者error_info.error_id为0时,表明没有错误
        """
        raise NotImplementedError

    def on_sub_market_data(self, ticker: dict, error_info: dict, is_last: bool):
        """
        订阅行情应答,包括股票、指数和期权
        *每条订阅的合约均对应一条订阅应答,需要快速返回,否则会堵塞后续消息,当堵塞严重时,会触发断线

        :param ticker: 详细的合约订阅情况
        :param error_info: 订阅合约发生错误时的错误信息,当error_info为空,或者error_info.error_id为0时,表明没有错误
        :param is_last: 是否此次订阅的最后一个应答,当为最后一个的时候为true,如果为false,表示还有其他后续消息响应
        """
        raise NotImplementedError

