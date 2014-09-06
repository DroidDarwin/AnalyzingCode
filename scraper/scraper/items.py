from scrapy.item import Item, Field

class ApkItem(Item):

    # Unique identifier, APK source, and collection date
    id              = Field()
    source_id       = Field()
    collection_date = Field()

    # App name, developer, and genre
    name        = Field()
    developer   = Field()
    genre       = Field()

    # User rating
    score       = Field()

    # Additional information
    date_published      = Field()
    file_size           = Field()
    num_downloads       = Field()
    lower_downloads     = Field()
    upper_downloads     = Field()
    software_version    = Field()
    operating_systems   = Field()

    # URL where the APK file came from
    url         = Field()

    # For Files Pipeline
    file_urls   = Field()
    files       = Field()
