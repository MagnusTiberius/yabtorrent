#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <arpa/inet.h>

#include "bencode.h"
#include "bt.h"
#include "bt_local.h"

static void __do_files_list(
    void *id,
    bencode_t * list
)
{
    while (bencode_list_has_next(list))
    {
        bencode_t dict;

        long int current_file_len = 0;

        bencode_list_get_next(list, &dict);

        while (bencode_dict_has_next(&dict))
        {
            bencode_t benk;

            const char *key;

            int klen;

            bencode_dict_get_next(&dict, &benk, &key, &klen);

            /*
             * 'length':
             * This is an integer indicating the total length of the file in
             * bytes.
             */
            if (!strncmp(key, "length", klen))
            {
                bencode_int_value(&benk, &current_file_len);
            }
            /*
             * 'md5sum':
             * This is an OPTIONAL value.if included
             * it must be a string of 32 characters corresponding to the MD5
             * sum of the file.This value is not used in BTP / 1.0.
             */
            if (!strncmp(key, "md5sum", klen))
            {

            }
            /*
             * 'path':
             * This is a list of string elements that specify the path of the
             * file, relative to the topmost directory.The last element in the
             * list is the name of the file, and the elements preceding it
             * indicate the directory hierarchy in which this file is situated.
             */
            else if (!strncmp(key, "path", klen))
            {
                char *fullpath = NULL;

                int fp_len = 0; // count including last nul byte

                while (bencode_list_has_next(&benk))
                {
                    const char *path;

                    bencode_t pathitem;

                    bencode_list_get_next(&benk, &pathitem);
                    bencode_string_value(&pathitem, &path, &klen);

                    if (!fullpath)
                    {
                        fullpath = strndup(path, klen);
                        fp_len = klen;
                    }
                    else
                    {
                        fullpath = realloc(fullpath, fp_len + klen + 2);
                        fullpath[fp_len] = '/';
                        strncpy(fullpath + fp_len + 1, path, klen);
                        fp_len += klen + 1;
                    }
                }

                bt_client_add_file(id, fullpath, fp_len, current_file_len);
                free(fullpath);
            }
        }
    }
}

static void __do_info_dict(
    void *id,
    bencode_t * dict,
    bt_piece_info_t * pinfo
)
{
#if 0
    bool got_length = FALSE;

    char *name = NULL;

    int name_len;
#endif

    {
        const char *val;

        int len;

        bencode_dict_get_start_and_len(dict, &val, &len);
        printf("hash: %d\n", len);
//        printf("hash: %d, %.*s\n", len, len, val, len);

        bt_client_set_info_hash(id, str2sha1hash(val, len));
        //bt_set_info_hash(id, str2sha1hash(val, len));
    }

    while (bencode_dict_has_next(dict))
    {
        const char *key;

        int klen;

        bencode_t benk;

        bencode_dict_get_next(dict, &benk, &key, &klen);

        /* is a single file torrent */
        if (!strncmp(key, "length", klen))
        {
            long int file_len;

            bencode_int_value(&benk, &file_len);
//            printf("got filelen %d\n", file_len);
//            bt_set_file_length(

            bencode_dict_get_next(dict, &benk, &key, &klen);
            if (!strncmp(key, "name", klen))
            {
                int len;

                const char *name;

                bencode_string_value(&benk, &name, &len);
                bt_client_add_file(id, name, len, file_len);
            }
        }
        /*  ...otherwise multi-file torrent */
        else if (!strncmp(key, "files", klen))
        {
            __do_files_list(id, &benk);
        }
#if 1
        else if (!strncmp(key, "piece length", klen))
        {
            int len;

            long int piece_len;

            piece_len = 0;
            bencode_int_value(&benk, &piece_len);
            bt_client_set_piece_length(id, piece_len);
//            pinfo->piece_len = len;
        }
#endif
        else if (!strncmp(key, "pieces", klen))
        {
            int len;

            const char *val;

            bencode_string_value(&benk, &val, &len);
            bt_client_add_pieces(id, val, len);
//            printf("%.*s\n", len, val);
        }
    }

}

/**
 *
 * This assigns the info_hash
 * */
void bt_client_read_metainfo(
    void *id,
    const char *buf,
    const int len,
    bt_piece_info_t * pinfo
//    const char *fname
)
{
//    bencode_next(&ben);
//    if (bencode_is_dict(&ben))
    bencode_t ben;

    bencode_init(&ben, buf, len);

    if (!bencode_is_dict(&ben))
    {
        return;
    }

    while (bencode_dict_has_next(&ben))
    {
        int klen;

        const char *key;

        bencode_t benk;

        bencode_dict_get_next(&ben, &benk, &key, &klen);

        if (!strncmp(key, "announce", klen))
        {
            int len;

            const char *val;

            bencode_string_value(&benk, &val, &len);
            bt_client_set_tracker_url(id, val, len);
        }
        else if (!strncmp(key, "announce-list", klen))
        {

            /*  loop through announce list */

            while (bencode_list_has_next(&benk))
            {
                bencode_t innerlist;


                bencode_list_get_next(&benk, &innerlist);
                while (bencode_list_has_next(&innerlist))
                {
                    bencode_t benlitem;

                    bencode_list_get_next(&innerlist, &benlitem);
                    const char *backup;

                    int len;

                    bencode_string_value(&benlitem, &backup, &len);
                    bt_client_add_tracker_backup(id, backup, len);
                }
            }
        }
        else if (!strncmp(key, "comment", klen))
        {
            int len;

            const char *val;

            bencode_string_value(&benk, &val, &len);
//            printf("comment: %.*s\n", len, val);
        }
        else if (!strncmp(key, "created by", klen))
        {
            int len;

            const char *val;

            bencode_string_value(&benk, &val, &len);
//            printf("created by: %.*s\n", len, val);
        }
        else if (!strncmp(key, "creation date", klen))
        {
            long int date;

            bencode_int_value(&benk, &date);
//            printf("created date: %ld\n", date);
        }
        else if (!strncmp(key, "encoding", klen))
        {
            int len;

            const char *val;

            bencode_string_value(&benk, &val, &len);
//            printf("encoding: %.*s\n", len, val);
        }
#if 1
        else if (!strncmp(key, "info", klen))
        {

//            printf("doing info:%s\n", benk.str);
            __do_info_dict(id, &benk, pinfo);
        }
#endif
    }
}