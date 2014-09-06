import re
import string
import requests
from scrapy import log
from scrapy.contrib.spiders import CrawlSpider, Rule
from scrapy.contrib.linkextractors.sgml import SgmlLinkExtractor
from scrapy.selector import Selector
from scrapy.http import Request
from scraper.items import ApkItem
from play import parse_app

class GooglePlaySpider(CrawlSpider):
	name = 'googleplay'
	start_urls = [
		'https://play.google.com/store/apps'
	]
	rules = (
		Rule(SgmlLinkExtractor(allow=('/store/apps$', )), callback='parse_category_group', follow=True),
		Rule(SgmlLinkExtractor(allow=('/store/apps/category/.*', )), callback='parse_category', follow=True),
		Rule(SgmlLinkExtractor(allow=('/store/search\?.*', )), callback='parse_search', follow=True),
	)

	def parse_category_group(self, response):
		sel = Selector(response)
		category_groups = sel.xpath('//div[@class="padded-content3 app-home-nav"]')

		for category_group in category_groups:

			category_group_name = category_group.xpath('h2/a/text()').extract()

			categories = category_group.xpath('ul/li')
			for category in categories:
				category_name = category.xpath('a/text()').extract()
				category_url = category.xpath('a/@href').extract()[0]

		chars = string.ascii_uppercase + string.digits
		for x in chars:
			yield Request('https://play.google.com/store/search?q=' + x + '&c=apps', callback=self.parse_search)

		for x in chars:
			for y in chars:
				yield Request('https://play.google.com/store/search?q=' + x + y + '&c=apps', callback=self.parse_search)

		for x in chars:
			for y in chars:
				for z in chars:
					yield Request('https://play.google.com/store/search?q=' + x + y + z + '&c=apps', callback=self.parse_search)        

		return

	def parse_category(self, response):
		base_path = response.url.split('?')[0]   

		if '/collection/' in response.url:
			sel = Selector(response)
			apps = sel.xpath('//a[@class="title"]')
			has_app = False

			for app in apps:
				has_app = True
				app_name = app.xpath('text()').extract()
				app_url = app.xpath('@href').extract()
				yield Request('https://play.google.com' + app_url[0], meta={'come_from': self.name}, callback=parse_app)

			if has_app:
				m = re.match(r'(.*)\?start=(\d+)&num=24', response.url)
				if m is None:
					start_number = 24                  
				else:
					start_number = int(m.group(2)) + 24
				yield Request(base_path + '?start=' + str(start_number) + '&num=24', callback=self.parse_category)

		return

	def parse_search(self, response):
		m = re.match(r'(.*)&start=(\d+)&num=24', response.url)
		if m is None:
			base_path = response.url
			start_number = 24                  
		else:
			start_number = int(m.group(2)) + 24
			base_path = m.group(1)

		sel = Selector(response)
		apps = sel.xpath('//a[contains(@href,"/store/apps/details")]')
		has_app = False

		for app in apps:
			has_app = True
			app_url = app.xpath('@href').extract()
			yield Request('https://play.google.com' + app_url[0], meta={'come_from': self.name}, callback=parse_app)

		if has_app:
			yield Request(base_path + '&start=' + str(start_number) + '&num=24', callback=self.parse_search)

		return
		