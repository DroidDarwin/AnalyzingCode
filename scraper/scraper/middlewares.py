import random
from scrapy import log

class RandomUserAgentMiddleware(object):
	def __init__(self, settings):
		self.user_agent_list = settings.get('USER_AGENT_LIST')

	@classmethod
	def from_crawler(cls, crawler):
		return cls(crawler.settings)

	def process_request(self, request, spider):
		ua = random.choice(self.user_agent_list)
		if ua:
			request.headers.setdefault('User-Agent', ua)

class ProxyMiddleware(object):
	def __init__(self, settings):
		self.proxy_list = settings.get('PROXY_LIST')

	@classmethod
	def from_crawler(cls, crawler):
		return cls(crawler.settings)

	def process_request(self, request, spider):
		proxy = random.choice(self.proxy_list)
		request.meta['proxy'] = proxy

	def process_exception(self, request, exception, spider):
		proxy = request.meta['proxy']
		log.msg('Removing failed proxy <%s>, %d proxies left' % (proxy, len(self.proxy_list)), level=log.WARNING)

		try:
			self.proxy_list.remove(proxy)
		except ValueError:
			pass
