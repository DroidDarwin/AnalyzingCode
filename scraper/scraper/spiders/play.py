import re
import uuid
from datetime import datetime
from scrapy import log
from scrapy.selector import Selector
from scraper.items import ApkItem

# Parse the information for an individual APK from the Google Play store
def parse_app(response):
    sel = Selector(response)
    item = ApkItem()

    # Creates a unique identifier for the APK
    item['id'] = str(uuid.uuid4())

    now = datetime.now()
    item['collection_date'] = now.strftime('%Y-%m-%d %H:%M:%S')

    # Special logic for GooglePlaySpider
    if response.meta['come_from'] == 'googleplay':

        item['source_id'] = 2

        price = sel.xpath('//meta[@itemprop="price"]/@content').extract()[0]

        # We are only downloading free apps
        if price != '0':
            log.msg('Not a free app, skipping item <%s>' % response.url, level=log.INFO)
            return

        item['url'] = response.url

    # Logic for all other Spider objects
    else:
        item['url'] = response.meta['url']
        item['file_urls'] = response.meta['file_urls']

        if response.meta['come_from'] == 'apktop':
            item['source_id'] = 3
        elif response.meta['come_from'] == 'teamapk':
            item['source_id'] = 4
        elif response.meta['come_from'] == 'fdroid':
            item['source_id'] = 5
        else:
            item['source_id'] = 1

    info_container = sel.xpath('//div[@class="info-container"]')
    item['name'] = info_container.xpath('//div[@class="document-title"]/div/text()').extract()[0]
    item['developer'] = info_container.xpath('//div[@itemprop="author"]/a/span[@itemprop="name"]/text()').extract()[0]
    item['genre'] = info_container.xpath('//span[@itemprop="genre"]/text()').extract()[0]

    score_container = sel.xpath('//div[@class="score-container"]')
    item['score'] = score_container.xpath('//div[@class="score"]/text()').extract()[0]

    additional_information = sel.xpath('//div[@class="details-section metadata"]')
    item['date_published'] = additional_information.xpath('//div[@itemprop="datePublished"]/text()').extract()[0]
    item['file_size'] = additional_information.xpath('//div[@itemprop="fileSize"]/text()').extract()[0].strip()
    item['num_downloads'] = additional_information.xpath('//div[@itemprop="numDownloads"]/text()').extract()[0].strip()
    item['lower_downloads'] = item['num_downloads'][0 : (item['num_downloads'].index("-")-1)].strip().replace(',', '')
    item['upper_downloads'] = item['num_downloads'][item['num_downloads'].index("-") + 1 : ].strip().replace(',', '')
    item['software_version'] = additional_information.xpath('//div[@itemprop="softwareVersion"]/text()').extract()[0].strip()
    item['operating_systems'] = additional_information.xpath('//div[@itemprop="operatingSystems"]/text()').extract()[0].strip()

    yield item
