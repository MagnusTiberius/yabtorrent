
//#include "bt_block_readwriter_i.h"

typedef int (
    *func_write_block_f
)   (
    void *udata,
    void *caller,
    const bt_block_t * blk,
    const void *blkdata
);

typedef void *(
    *func_read_block_f
)    (
    void *udata,
    void *caller,
    const bt_block_t * blk
);

typedef void *(
    *func_add_file_f
)    (
    void *caller,
    const char *fname,
    const int size
);

typedef void (
    *func_log_f
)    (
    void *udata,
    void *src,
//    bt_peer_t * peer,
    const char *buf,
    ...
);

typedef struct
{
    func_write_block_f write_block;

    func_read_block_f read_block;

    /*  release this block from the holder of it */
//    func_giveup_block_f giveup_block;
} bt_blockrw_i;

/*  piece info
 *  this is how this torrent has */
typedef struct
{
    /* a string containing the 20 byte sha1 of every file, concatenated.
     * (from protocol)
     * sha1 hash = 20 bytes */
    char *pieces_hash;
    /* the length of a piece (from protocol) */
    int piece_len;
    /* number of pieces (from protocol) */
    int npieces;
} bt_piece_info_t;

/** cfg */
typedef struct
{
    int select_timeout_msec;
    int max_peer_connections;
    int max_active_peers;
    int max_cache_mem;
//    int tracker_scrape_interval;
    /*  don't seed, shutdown when complete */
    int o_shutdown_when_complete;
    /*  the size of the piece, etc */
    bt_piece_info_t pinfo;
    /*  how many seconds between tracker scrapes */
//    int o_tracker_scrape_interval;
    /* listen for pwp messages on this port */
    int pwp_listen_port;
    /*  this holds my IP. I figure it out */
    char my_ip[32];
    /* sha1 hash of the info_hash */
    char *info_hash;
    /* 20-byte self-designated ID of the peer */
    char *p_peer_id;

//    char *tracker_url;
} bt_client_cfg_t;

typedef struct
{
    /**
     * Connect to the peer
     * @param udata use this memory for the connection. It is up to the callee to alloc memory.
     * @param host the hostname
     * @param port the host's port
     * @param peerid pointer available for the callee to identify the peer */
    int (*peer_connect) (void **udata,
                         const char *host, const char *port, int *peerid);

    int (*peer_send) (void **udata,
                      const int peerid,
                      const unsigned char *send_data, const int len);

    int (*peer_recv_len) (void **udata, int peerid, char *recv, int *len);

    int (*peer_disconnect) (void **udata, int peerid);

    int (*peers_poll) (void **udata,
                       const int msec_timeout,
                       int (*func_process) (void *,
                                            int),
                       void (*func_process_connection) (void *,
                                                        int netid,
                                                        char *ip,
                                                        int), void *data);

    int (*peer_listen_open) (void **udata, const int port);

} bt_client_funcs_t;


/*  bittorrent piece */
typedef struct
{
    /* index on 'bit stream' */
    const int idx;

} bt_piece_t;

/* peer */
typedef struct
{
    /* 20 byte sha1 string */
    char *peer_id;
    char *ip;
    char *port;

    /* for network api */
    int net_peerid;

    /* peer connection */
    void* pc;
} bt_peer_t;


char *bt_generate_peer_id();

void *bt_client_new();

void *bt_client_get_piece(void *bto, const unsigned int piece_idx);

int bt_client_get_num_peers(void *bto);

int bt_client_get_num_pieces(void *bto);

int bt_client_get_total_file_size(void *bto);

char *bt_client_get_fail_reason(void *bto);

int bt_client_get_nbytes_downloaded(void *bto);

int bt_client_is_failed(void *bto);

void *bt_client_add_peer(void *bto,
                              const char *peer_id,
                              const int peer_id_len,
                              const char *ip, const int ip_len, const int port);

void* bt_client_get_config(void *bto);