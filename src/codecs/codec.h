/* 
 * File:   codec.h
 * Author: Moddus
 *
 * Created on 15. August 2013, 12:24
 */

#ifndef CODEC_H
#define	CODEC_H


/**
 * Constant to identify the start of a codec header.
 */
#define LCN_CODEC_MAGIC ( 0x3fd76c17 )

/**
 * Writes a codec header, which records both a string to
 * identify the file and a version number. This header can
 * be parsed and validated with 
 * {@link #checkHeader(DataInput, String, int, int) checkHeader()}.
 * <p>
 * CodecHeader --&gt; Magic,CodecName,Version
 * <ul>
 *    <li>Magic --&gt; {@link DataOutput#writeInt Uint32}. This
 *        identifies the start of the header. It is always {@value #CODEC_MAGIC}.
 *    <li>CodecName --&gt; {@link DataOutput#writeString String}. This
 *        is a string to identify this file.
 *    <li>Version --&gt; {@link DataOutput#writeInt Uint32}. Records
 *        the version of the file.
 * </ul>
 * <p>
 * Note that the length of a codec header depends only upon the
 * name of the codec, so this length can be computed at any time
 * with {@link #headerLength(String)}.
 * 
 * @param out Output stream
 * @param codec String to identify this file. It should be simple ASCII, 
 *              less than 128 characters in length.
 * @param version Version number
 * @throws IOException If there is an I/O error writing to the underlying medium.
 */
apr_status_t
lcn_codec_util_write_header( lcn_index_output_t *out, 
                             char* codec, 
                             unsigned int version );

#endif	/* CODEC_H */

