from scrapy import log
from scrapy.contrib.spiders import CrawlSpider
from scrapy.selector import Selector
from scrapy.http import Request
from scrapy.http import FormRequest
from play import parse_app

class TeamApkSpider(CrawlSpider):
    name = "teamapk"
    start_urls = ["http://www.teamapk.co"]

    # Parses the Team APK home page and subsequent pages
    def parse(self, response):
        sel = Selector(response)
        apk_page_urls = sel.xpath('//h1/a[contains(@href, "android-download")]/@href').extract()

        for url in apk_page_urls:
            yield Request(url, callback=self.parse_page)

        next_page = sel.xpath('//a[@class="nextpostslink"]/@href').extract()
        
        try:
            yield Request(next_page[0])
        except IndexError:
            log.msg('Crawling last page, spider will close soon', level=log.INFO)
            return

    # Parses pages for individual APK files
    def parse_page(self, response):
        sel = Selector(response)
        download_url = sel.xpath('//p/a[".apk" = substring(@href, string-length(@href) - 3)]/@href').extract()
        google_play_url = sel.xpath('//p/a[contains(@href, "play.google.com/store/apps")]/@href').extract()
        
        if download_url and google_play_url:
            yield Request(download_url[0], meta={'url': response.url, 'google_play_url': google_play_url[0]}, callback=self.parse_file)

    # Parses the URL to an actual APK file
    def parse_file(self, response):
        return [FormRequest.from_response(response, method="POST", meta={'url': response.meta['url'], 'google_play_url': response.meta['google_play_url']}, callback=self.after_post)]

    # Download the APK file
    def after_post(self, response):
        yield Request(response.meta['google_play_url'], meta={'url': response.meta['url'], 'file_urls': [response.url], 'come_from': self.name}, callback=parse_app)
        