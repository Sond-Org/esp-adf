/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2018 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on all ESPRESSIF SYSTEMS products, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef _HTTP_STREAM_H_
#define _HTTP_STREAM_H_

#include "audio_error.h"
#include "audio_element.h"
#include "audio_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief      HTTP Stream hook type
 */
typedef enum {
    HTTP_STREAM_PRE_REQUEST = 0x01, /*!< The event handler will be called before HTTP Client making the connection to the server.
                                     * Sond: called before every request, with the client's URL already set: the first one,
                                     * a reconnect, the target of a redirect and the next track of a playlist */
    HTTP_STREAM_ON_REQUEST,         /*!< The event handler will be called when HTTP Client is requesting data,
                                     * If the fucntion return the value (-1: ESP_FAIL), HTTP Client will be stopped
                                     * If the fucntion return the value > 0, HTTP Stream will ignore the post_field
                                     * If the fucntion return the value = 0, HTTP Stream continue send data from post_field (if any)
                                     */
    HTTP_STREAM_ON_RESPONSE,        /*!< The event handler will be called when HTTP Client is receiving data
                                     * If the fucntion return the value (-1: ESP_FAIL), HTTP Client will be stopped
                                     * If the fucntion return the value > 0, HTTP Stream will ignore the read function
                                     * If the fucntion return the value = 0, HTTP Stream continue read data from HTTP Server
                                     */
    HTTP_STREAM_POST_REQUEST,       /*!< The event handler will be called after HTTP Client send header and body to the server, before fetching the headers */
    HTTP_STREAM_FINISH_REQUEST,     /*!< The event handler will be called after HTTP Client fetch the header and ready to read HTTP body */
    HTTP_STREAM_RESOLVE_ALL_TRACKS,
    HTTP_STREAM_FINISH_TRACK,
    HTTP_STREAM_FINISH_PLAYLIST,
    HTTP_STREAM_ON_HEADERS,         /*!< Sond: after each request's response headers were read, buffer_len = the HTTP status,
                                     * or after a request that failed before them, buffer_len = -1. The return value is ignored */
    HTTP_STREAM_ON_REDIRECT,        /*!< Sond: before following a 301, 302, 303, 307 or 308. buffer = a writable, NUL-terminated
                                     * copy of the Location, buffer_len = its capacity; the hook may rewrite it in place.
                                     * Return -1 (ESP_FAIL) to fail the request */
} http_stream_event_id_t;

/**
 * @brief      Stream event message
 */
typedef struct {
    http_stream_event_id_t  event_id;       /*!< Event ID */
    void                    *http_client;   /*!< Reference to HTTP Client using by this HTTP Stream */
    void                    *buffer;        /*!< Reference to Buffer using by the Audio Element */
    int                     buffer_len;     /*!< Length of buffer */
    void                    *user_data;     /*!< User data context, from `http_stream_cfg_t` */
    audio_element_handle_t  el;             /*!< Audio element context */
} http_stream_event_msg_t;

typedef int (*http_stream_event_handle_t)(http_stream_event_msg_t *msg);

/**
 * @brief      HTTP Stream configurations
 *             Default value will be used if any entry is zero
 */
typedef struct {
    audio_stream_type_t         type;                   /*!< Type of stream */
    int                         out_rb_size;            /*!< Size of output ringbuffer */
    int                         task_stack;             /*!< Task stack size */
    int                         task_core;              /*!< Task running in core (0 or 1) */
    int                         task_prio;              /*!< Task priority (based on freeRTOS priority) */
    bool                        stack_in_ext;           /*!< Try to allocate stack in external memory */
    http_stream_event_handle_t  event_handle;           /*!< The hook function for HTTP Stream */
    void                        *user_data;             /*!< User data context */
    bool                        auto_connect_next_track;/*!< connect next track without open/close */
    bool                        enable_playlist_parser; /*!< Enable playlist parser*/
    int                         multi_out_num;          /*!< The number of multiple output */
    const char                  *cert_pem;              /*!< SSL server certification, PEM format as string, if the client requires to verify server */
    esp_err_t (*crt_bundle_attach)(void *conf);         /*!< Function pointer to esp_crt_bundle_attach. Enables the use of certification
                                                             bundle for server verification, must be enabled in menuconfig */
    int                         request_size;           /*!< Request data size each time from `http_client`
                                                             Defaults use DEFAULT_ELEMENT_BUFFER_LENGTH if set to 0
                                                             Need care this setting if audio frame size is small and want low latency playback */                                                         
    int                         request_range_size;     /*!< Range size setting for header `Range: bytes=start-end`
                                                             Request full range of resource if set to 0
                                                             Range size bigger than request size is recommended */
    const char                  *user_agent;            /*!< The User Agent string to send with HTTP requests */
    int                         reconnect_window_ms;    /*!< After a connection loss in the middle of a stream, keep retrying
                                                             transport failures until they have used this much time with the
                                                             network up (each attempt is capped at the budget left, and an HTTP
                                                             4xx gives up at once). 0 makes a single immediate attempt, as
                                                             upstream ADF does */
    int                         reconnect_wait_max_ms;  /*!< Cap on the whole retry, including time spent waiting for the
                                                             network to come back. Values below reconnect_window_ms mean
                                                             reconnect_window_ms */
    bool                        (*network_ready)(void); /*!< Optional. Returns false while the network is down; the retry then
                                                             waits without attempting and without using reconnect_window_ms.
                                                             NULL means the network always counts as up */
} http_stream_cfg_t;

#define HTTP_STREAM_TASK_STACK          (6 * 1024)
#define HTTP_STREAM_TASK_CORE           (0)
#define HTTP_STREAM_TASK_PRIO           (4)
#define HTTP_STREAM_RINGBUFFER_SIZE     (20 * 1024)

#define HTTP_STREAM_CFG_DEFAULT() {              \
    .type = AUDIO_STREAM_READER,                 \
    .out_rb_size = HTTP_STREAM_RINGBUFFER_SIZE,  \
    .task_stack = HTTP_STREAM_TASK_STACK,        \
    .task_core = HTTP_STREAM_TASK_CORE,          \
    .task_prio = HTTP_STREAM_TASK_PRIO,          \
    .stack_in_ext = true,                        \
    .event_handle = NULL,                        \
    .user_data = NULL,                           \
    .auto_connect_next_track = false,            \
    .enable_playlist_parser = false,             \
    .multi_out_num = 0,                          \
    .cert_pem  = NULL,                           \
    .crt_bundle_attach = NULL,                   \
    .user_agent = NULL,                          \
    .reconnect_window_ms = 0,                    \
    .reconnect_wait_max_ms = 0,                  \
    .network_ready = NULL,                       \
}

/**
 * @brief      Create a handle to an Audio Element to stream data from HTTP to another Element
 *             or get data from other elements sent to HTTP, depending on the configuration
 *             the stream type, either AUDIO_STREAM_READER or AUDIO_STREAM_WRITER.
 *
 * @param      config  The configuration
 *
 * @return     The Audio Element handle
 */
audio_element_handle_t http_stream_init(http_stream_cfg_t *config);

/**
 * @brief      Connect to next track in the playlist.
 *
 *             This function can be used in event_handler of http_stream.
 *             User can call this function to connect to next track in playlist when he/she gets `HTTP_STREAM_FINISH_TRACK` event
 *
 * @param      el  The http_stream element handle
 *
 * @return
 *     - ESP_OK on success
 *     - ESP_FAIL on errors
 */
esp_err_t http_stream_next_track(audio_element_handle_t el);
esp_err_t http_stream_restart(audio_element_handle_t el);

/**
 * @brief       Try to fetch the tracks again.
 *
 *              If this is live stream we will need to keep fetching URIs.
 *
 * @param       el  The http_stream element handle
 *
 * @return
 *     - ESP_OK on success
 *     - ESP_ERR_NOT_SUPPORTED if playlist is finished
 */
esp_err_t http_stream_fetch_again(audio_element_handle_t el);

/**
 * @brief       Set SSL server certification
 * @note        EM format as string, if the client requires to verify server
 *
 * @param       el    The http_stream element handle
 * @param       cert  server certification
 *
 * @return
 *     - ESP_OK on success
 */
esp_err_t http_stream_set_server_cert(audio_element_handle_t el, const char *cert);

/**
 * @brief      Sond: open the next track at `url` instead of walking its stream URI's redirects, e.g. the final URL
 *             of a podcast enclosure's chain resolved ahead of time. Used once, by the next open at byte 0; it then
 *             serves as the stream's redirect target for mid-track reconnects. If that open fails for any reason the
 *             stream URI is opened as usual. Pass NULL to clear an unused one (call it after the play returns).
 *
 * @param       el    The http_stream element handle
 * @param       url   The target (copied), or NULL
 *
 * @return
 *     - ESP_OK on success
 *     - ESP_ERR_NO_MEM when the copy fails (nothing is set)
 */
esp_err_t http_stream_set_first_target(audio_element_handle_t el, const char *url);

#ifdef __cplusplus
}
#endif

#endif
