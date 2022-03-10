/**
 * @file core.h  Internal API
 *
 * Copyright (C) 2010 Alfred E. Heggestad
 */


#include <limits.h>


/* max bytes in pathname */
#if defined (PATH_MAX)
#define FS_PATH_MAX PATH_MAX
#elif defined (_POSIX_PATH_MAX)
#define FS_PATH_MAX _POSIX_PATH_MAX
#else
#define FS_PATH_MAX 512
#endif


/** Media constants */
enum {
	AUDIO_BANDWIDTH = 128000,  /**< Bandwidth for audio in bits/s      */
	VIDEO_SRATE     =  90000,  /**< Sampling rate for video            */
};


/* forward declarations */
struct stream_param;


/*
 * Account
 */


struct account {
	char *buf;                   /**< Buffer for the SIP address         */
	struct sip_addr laddr;       /**< Decoded SIP address                */
	struct uri luri;             /**< Decoded AOR uri                    */
	char *dispname;              /**< Display name                       */
	char *aor;                   /**< Local SIP uri                      */

	/* parameters: */
	bool sipans;                 /**< Allow SIP header auto answer mode  */
	enum sipansbeep sipansbeep;  /**< Beep mode for SIP auto answer      */
	enum answermode answermode;  /**< Answermode for incoming calls      */
	int32_t adelay;              /**< Delay for delayed auto answer [ms] */
	enum dtmfmode dtmfmode;      /**< Send type for DTMF tones           */
	struct le acv[16];           /**< List elements for aucodecl         */
	struct list aucodecl;        /**< List of preferred audio-codecs     */
	char *auth_user;             /**< Authentication username            */
	char *auth_pass;             /**< Authentication password            */
	char *mnatid;                /**< Media NAT handling                 */
	char *mencid;                /**< Media encryption type              */
	const struct mnat *mnat;     /**< MNAT module                        */
	const struct menc *menc;     /**< MENC module                        */
	char *outboundv[2];          /**< Optional SIP outbound proxies      */
	uint32_t ptime;              /**< Configured packet time in [ms]     */
	uint32_t txptime;            /**< Configured TX packet time in [ms]  */
	uint32_t regint;             /**< Registration interval in [seconds] */
	uint32_t fbregint;           /**< Fallback R. interval in [seconds]  */
	uint32_t rwait;              /**< R. Int. in [%] from proxy expiry   */
	uint32_t pubint;             /**< Publication interval in [seconds]  */
	uint32_t prio;               /**< Prio for serial registration       */
	char *regq;                  /**< Registration Q-value               */
	char *sipnat;                /**< SIP Nat mechanism                  */
	char *stun_user;             /**< STUN Username                      */
	char *stun_pass;             /**< STUN Password                      */
	struct stun_uri *stun_host;  /**< STUN Server                        */
	struct le vcv[4];            /**< List elements for vidcodecl        */
	struct list vidcodecl;       /**< List of preferred video-codecs     */
	bool videoen;                /**< Video enabled flag                 */
	bool mwi;                    /**< MWI on/off                         */
	bool refer;                  /**< REFER method on/off                */
	char *cert;                  /**< SIP TLS client certificate+keyfile */
	char *ausrc_mod;
	char *ausrc_dev;
	char *auplay_mod;
	char *auplay_dev;
	uint32_t autelev_pt;         /**< Payload type for telephone-events  */
	uint32_t dtx_supported;
	char *extra;                 /**< Extra parameters                   */
};


/*
 * Audio Stream
 */

struct audio;

int  audio_send_digit(struct audio *a, char key);
void audio_sdp_attr_decode(struct audio *a);


/*
 * Call Control
 */

enum {
	CALL_LINENUM_MIN  =   1,
	CALL_LINENUM_MAX  = 256
};

struct call;

/** Call parameters */
struct call_prm {
	struct sa laddr;
	enum vidmode vidmode;
	int af;
	bool use_rtp;
};

int  call_alloc(struct call **callp, const struct config *cfg,
		struct list *lst,
		const char *local_name, const char *local_uri,
		struct account *acc, struct ua *ua, const struct call_prm *prm,
		const struct sip_msg *msg, struct call *xcall,
		struct dnsc *dnsc,
		call_event_h *eh, void *arg);
int  call_accept(struct call *call, struct sipsess_sock *sess_sock,
		 const struct sip_msg *msg);
int  call_sdp_get(const struct call *call, struct mbuf **descp, bool offer);
int  call_info(struct re_printf *pf, const struct call *call);
int  call_reset_transp(struct call *call, const struct sa *laddr);
int  call_af(const struct call *call);
void call_set_xrtpstat(struct call *call);
void call_set_custom_hdrs(struct call *call, const struct list *hdrs);
const struct sa *call_laddr(const struct call *call);

/*
* Custom headers
*/
int custom_hdrs_print(struct re_printf *pf,
		       const struct list *custom_hdrs);

/*
 * Conf
 */

int conf_get_csv(const struct conf *conf, const char *name,
		 char *str1, size_t sz1, char *str2, size_t sz2);
int conf_get_float(const struct conf *conf, const char *name, double *val);


/*
 * Metric
 */

struct metric {
	/* internal stuff: */
	struct tmr tmr;
	struct lock *lock;
	uint64_t ts_start;
	bool started;

	/* counters: */
	uint32_t n_packets;
	uint32_t n_bytes;
	uint32_t n_err;

	/* bitrate calculation */
	uint32_t cur_bitrate;
	uint64_t ts_last;
	uint32_t n_bytes_last;
};

int      metric_init(struct metric *metric);
void     metric_reset(struct metric *metric);
void     metric_add_packet(struct metric *metric, size_t packetsize);
double   metric_avg_bitrate(const struct metric *metric);


/*
 * Module
 */

int module_init(const struct conf *conf);


/*
 * Register client
 */

struct reg;

int  reg_add(struct list *lst, struct ua *ua, int regid);
int  reg_register(struct reg *reg, const char *reg_uri,
		    const char *params, uint32_t regint, const char *outbound);
void reg_unregister(struct reg *reg);
bool reg_isok(const struct reg *reg);
bool reg_failed(const struct reg *reg);
int  reg_debug(struct re_printf *pf, const struct reg *reg);
int  reg_json_api(struct odict *od, const struct reg *reg);
int  reg_status(struct re_printf *pf, const struct reg *reg);
int  reg_af(const struct reg *reg);
const struct sa *reg_laddr(const struct reg *reg);


/*
 * RTP Header Extensions
 */

#define RTPEXT_HDR_SIZE        4
#define RTPEXT_TYPE_MAGIC 0xbede

enum {
	RTPEXT_ID_MIN  =  1,
	RTPEXT_ID_MAX  = 14,
};

enum {
	RTPEXT_LEN_MIN =  1,
	RTPEXT_LEN_MAX = 16,
};

struct rtpext {
	unsigned id:4;
	unsigned len:4;
	uint8_t data[RTPEXT_LEN_MAX];
};


int rtpext_hdr_encode(struct mbuf *mb, size_t num_bytes);
int rtpext_encode(struct mbuf *mb, unsigned id, size_t len,
		  const uint8_t *data);
int rtpext_decode(struct rtpext *ext, struct mbuf *mb);


/*
 * RTP Stats
 */

int rtpstat_print(struct re_printf *pf, const struct call *call);


/*
 * SDP
 */

int sdp_decode_multipart(const struct pl *ctype_prm, struct mbuf *mb);


/* bundle (per media stream) */


enum bundle_state {
	BUNDLE_NONE = 0,
	BUNDLE_BASE,
	BUNDLE_MUX
};

struct bundle;

int  bundle_alloc(struct bundle **bunp);
void bundle_handle_extmap(struct bundle *bun, struct sdp_media *sdp);
int  bundle_start_socket(struct bundle *bun, struct udp_sock *us,
			 struct list *streaml);
enum bundle_state bundle_state(const struct bundle *bun);
uint8_t bundle_extmap_mid(const struct bundle *bun);
int bundle_set_extmap(struct bundle *bun, struct sdp_media *sdp,
		      uint8_t extmap_mid);
void bundle_set_state(struct bundle *bun, enum bundle_state st);
int  bundle_debug(struct re_printf *pf, const struct bundle *bun);


const char *bundle_state_name(enum bundle_state st);


/*
 * Stream
 */

enum media_type {
	MEDIA_AUDIO = 0,
	MEDIA_VIDEO,
};

struct sender;
struct receiver;
struct stream;
struct rtp_header;

enum {STREAM_PRESZ = 4+12}; /* same as RTP_HEADER_SIZE */

typedef void (stream_rtp_h)(const struct rtp_header *hdr,
			    struct rtpext *extv, size_t extc,
			    struct mbuf *mb, unsigned lostc, bool *ignore,
			    void *arg);
typedef int (stream_pt_h)(uint8_t pt, struct mbuf *mb, void *arg);


int  stream_alloc(struct stream **sp, struct list *streaml,
		  const struct stream_param *prm,
		  const struct config_avt *cfg,
		  struct sdp_session *sdp_sess,
		  enum media_type type,
		  const struct mnat *mnat, struct mnat_sess *mnat_sess,
		  const struct menc *menc, struct menc_sess *menc_sess,
		  bool offerer,
		  stream_rtp_h *rtph, stream_rtcp_h *rtcph, stream_pt_h *pth,
		  void *arg);
void stream_hold(struct stream *s, bool hold);
void stream_set_ldir(struct stream *s, enum sdp_dir dir);
void stream_set_srate(struct stream *s, uint32_t srate_tx, uint32_t srate_rx);
bool stream_is_ready(const struct stream *strm);
int  stream_print(struct re_printf *pf, const struct stream *s);
enum media_type stream_type(const struct stream *strm);
enum sdp_dir stream_ldir(const struct stream *s);
struct rtp_sock *stream_rtp_sock(const struct stream *strm);
const struct sa *stream_raddr(const struct stream *strm);
const char *stream_mid(const struct stream *strm);
uint8_t stream_generate_extmap_id(struct stream *strm);

/* Send */
void stream_update_encoder(struct stream *s, int pt_enc);
int  stream_pt_enc(const struct stream *strm);
int  stream_send(struct stream *s, bool ext, bool marker, int pt, uint32_t ts,
		 struct mbuf *mb);

/* Receive */
void stream_flush(struct stream *s);
void stream_silence_on(struct stream *s, bool on);
int  stream_decode(struct stream *s);
int  stream_ssrc_rx(const struct stream *strm, uint32_t *ssrc);


struct bundle *stream_bundle(const struct stream *strm);
void stream_parse_mid(struct stream *strm);
void stream_enable_bundle(struct stream *strm, enum bundle_state st);


/*
 * User-Agent
 */

struct ua;

void         ua_printf(const struct ua *ua, const char *fmt, ...);

int ua_print_allowed(struct re_printf *pf, const struct ua *ua);
struct call *ua_find_call_onhold(const struct ua *ua);
struct call *ua_find_active_call(struct ua *ua);
void ua_handle_options(struct ua *ua, const struct sip_msg *msg);
void sipsess_conn_handler(const struct sip_msg *msg, void *arg);
bool ua_catchall(struct ua *ua);
bool ua_reghasladdr(const struct ua *ua, const struct sa *laddr);

/*
 * User-Agent Group
 */

struct uag {
	struct config_sip *cfg;        /**< SIP configuration               */
	struct list ual;               /**< List of User-Agents (struct ua) */
	struct sip *sip;               /**< SIP Stack                       */
	struct sip_lsnr *lsnr;         /**< SIP Listener                    */
	struct sipsess_sock *sock;     /**< SIP Session socket              */
	struct sipevent_sock *evsock;  /**< SIP Event socket                */
	uint32_t transports;           /**< Supported transports mask       */
	bool delayed_close;            /**< Module will close SIP stack     */
	sip_msg_h *subh;               /**< Subscribe handler               */
	ua_exit_h *exith;              /**< UA Exit handler                 */
	bool nodial;                   /**< Prevent outgoing calls          */
	bool dnd;                      /**< Do not Disturb flag             */
	void *arg;                     /**< UA Exit handler argument        */
	char *eprm;                    /**< Extra UA parameters             */
#ifdef USE_TLS
	struct tls *tls;               /**< TLS Context                     */
	struct tls *wss_tls;           /**< Secure websocket TLS Context    */
#endif
};

struct config_sip *uag_cfg(void);
const char *uag_eprm(void);
bool uag_delayed_close(void);
int uag_raise(struct ua *ua, struct le *le);

void u32mask_enable(uint32_t *mask, uint8_t bit, bool enable);
bool u32mask_enabled(uint32_t mask, uint8_t bit);


/*
 * Video Stream
 */

struct video;

int  video_decoder_set(struct video *v, struct vidcodec *vc, int pt_rx,
		       const char *fmtp);
int  video_print(struct re_printf *pf, const struct video *v);


/*
 * Timestamp helpers
 */


/**
 * This struct is used to keep track of timestamps for
 * incoming RTP packets.
 */
struct timestamp_recv {
	uint32_t first;
	uint32_t last;
	bool is_set;
	unsigned num_wraps;
};


int      timestamp_wrap(uint32_t ts_new, uint32_t ts_old);
void     timestamp_set(struct timestamp_recv *ts, uint32_t rtp_ts);
uint64_t timestamp_duration(const struct timestamp_recv *ts);
uint64_t timestamp_calc_extended(uint32_t num_wraps, uint32_t ts);
double   timestamp_calc_seconds(uint64_t ts, uint32_t clock_rate);
