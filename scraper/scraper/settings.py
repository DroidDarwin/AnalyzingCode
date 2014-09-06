BOT_NAME = 'scraper'

SPIDER_MODULES = ['scraper.spiders']
NEWSPIDER_MODULE = 'scraper.spiders'

ITEM_PIPELINES = {
	'scraper.pipelines.SQLiteStorePipeline' : 300,
	'scraper.pipelines.GooglePlayDownloadPipeline' : 500,
	'scraper.pipelines.APKFilesPipeline' : 800,
}

DOWNLOADER_MIDDLEWARES = {
	'scraper.middlewares.RandomUserAgentMiddleware' : 200,
	'scraper.middlewares.ProxyMiddleware' : None,
	'scrapy.contrib.downloadermiddleware.useragent.UserAgentMiddleware' : None,
}

FILES_STORE = './downloads'

DOWNLOAD_DELAY = 0.25

COOKIES_ENABLED = False

# Need to be proxies from the USA to gather data in English
PROXY_LIST = [
	'http://76.72.217.131:8080'
]

USER_AGENT_LIST = [
	'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_6_8) AppleWebKit/537.13+ (KHTML, like Gecko) Version/5.1.7 Safari/534.57.2',
	'Mozilla/5.0 (Windows; U; Windows NT 5.0; en-US; rv:1.4b) Gecko/20030516 Mozilla Firebird/0.6',
	'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_9_2) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/36.0.1944.0 Safari/537.36',
	'Mozilla/5.0 (Windows NT 5.1; rv:31.0) Gecko/20100101 Firefox/31.0',
	'Mozilla/5.0 (X11; Linux x86_64; rv:28.0) Gecko/20100101 Firefox/28.0'
]

# Crawl responsibly by identifying yourself (and your website) on the user-agent
# USER_AGENT = 'APK File Scraper (+https://github.com/amb8805/ProjectKrutz)'
