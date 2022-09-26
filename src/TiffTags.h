#ifndef TIFF_TAGS_H
#define TIFF_TAGS_H

/* Bilevel image tags */
#define TAG_PHOTOMETRIC_INTERPRETATION 262
#define TAG_COMPRESSION 259
#define TAG_IMAGE_WIDTH 256
#define TAG_IMAGE_LENGTH 257
#define TAG_RESOLUTION_UNIT 296
#define TAG_X_RESOLUTION 282
#define TAG_Y_RESOLUTION 283
#define TAG_STRIP_OFFSETS 273
#define TAG_ROWS_PER_STRIP 278
#define TAG_STRIP_BYTE_COUNTS 279

/* Grayscale image tags (includes all above) */
#define TAG_BITS_PER_SAMPLE 258

/* Palette image tags (includes all above) */
#define TAG_COLOR_MAP 320

/* RGB image tags (includes all above) */
#define TAG_SAMPLES_PER_PIXEL 277

/* Baseline TIFF fields */
#define TAG_SOFTWARE 305
#define TAG_ARTIST 315
#define TAG_PLANAR_CONFIGURATION 284
#define TAG_THRESHOLDING 263
#define TAG_CELL_WIDTH 264
#define TAG_CELL_LENGTH 265
#define TAG_FILL_ORDER 266
#define TAG_COPYRIGHT 33432
#define TAG_DATE_TIME 306
#define TAG_HOST_COMPUTER 316
#define TAG_EXTRA_SAMPLES 338
#define TAG_FREE_OFFSETS 288
#define TAG_FREE_BYTE_COUNTS 289
#define TAG_GRAY_RESPONSE_UNIT 290
#define TAG_GRAY_RESPONSE_CURVE 291
#define TAG_NEW_SUBFILE_TYPE 254
#define TAG_SUBFILE_TYPE 255
#define TAG_IMAGE_DESCRIPTION 270
#define TAG_MAKE 271
#define TAG_MODEL 272
#define TAG_OREINTATION 274
#define TAG_MIN_SAMPLE_VALUE 280
#define TAG_MAX_SAMPLE_VALUE 281

#endif