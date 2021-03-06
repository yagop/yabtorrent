
/**
 * Copyright (c) 2011, Willem-Hendrik Thiart
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * @file
 * @author  Willem Thiart himself@willemthiart.com
 * @version 0.1
 */

#include <CuTest.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <assert.h>

#include "bt.h"
#include "bt_string.h"
#include "network_adapter.h"
#include "network_adapter_mock.h"
#include "mock_torrent.h"
#include "mock_client.h"

#include "bt_piece_db.h"
#include "bt_diskmem.h"
#include "config.h"
#include "linked_list_hashmap.h"
#include "bipbuffer.h"

#include <fcntl.h>
#include <sys/time.h>

void TestBT_Peer_shares_one_piece(
    CuTest * tc
    )
{
    int ii;
    client_t* a, *b;
    hashmap_iterator_t iter;
    void* mt;
    char *addr;

    clients_setup();
    mt = mocktorrent_new(1, 5);
    a = mock_client_setup(5);
    b = mock_client_setup(5);

    for (
        hashmap_iterator(clients_get(), &iter);
        hashmap_iterator_has_next(clients_get(), &iter);
        )
    {
        char hash[21];

        client_t* cli = hashmap_iterator_next_value(clients_get(), &iter);
        void* bt = cli->bt;
        void *cfg = bt_dm_get_config(bt);
        /* default configuration for clients */
        config_set(cfg, "npieces", "1");
        config_set_va(cfg, "piece_length", "%d", 5);
        config_set(cfg, "infohash", "00000000000000000000");
        /* add files/pieces */
        //bt_piecedb_add_file(bt_dm_get_piecedb(bt),"test.txt",8,5);
        bt_piecedb_increase_piece_space(bt_dm_get_piecedb(bt), 5);
        bt_piecedb_add_with_hash_and_size(bt_dm_get_piecedb(bt),
                                          mocktorrent_get_piece_sha1(mt, hash,
                                                                     0), 5);
    }

    /* write blocks to client A */
    {
        void* data;
        bt_block_t blk;

        data = mocktorrent_get_data(mt, 0);
        blk.piece_idx = 0;
        blk.offset = 0;
        blk.len = 5;
        bt_diskmem_write_block(
            bt_piecedb_get_diskstorage(bt_dm_get_piecedb(a->bt)),
            NULL, &blk, data);
    }

    bt_dm_check_pieces(a->bt);
    bt_dm_check_pieces(b->bt);
    /* let validation jobs run */
    bt_dm_periodic(a->bt, NULL);
    CuAssertTrue(tc, 1 ==
                 bt_piecedb_all_pieces_are_complete(bt_dm_get_piecedb(a->bt)));

    /* connect clients */
    asprintf(&addr, "%p", a);
    client_add_peer(b, NULL, 0, addr, strlen(addr), 0);

    for (ii = 0; ii < 10; ii++)
    {
#if 0   /* debugging */
        printf("\nStep %d:\n", ii + 1);
#endif

        bt_dm_periodic(a->bt, NULL);
        bt_dm_periodic(b->bt, NULL);

        network_poll(a->bt, (void*)&a, 0,
                     bt_dm_dispatch_from_buffer,
                     mock_on_connect);

        network_poll(b->bt, (void*)&b, 0,
                     bt_dm_dispatch_from_buffer,
                     mock_on_connect);

//        __print_client_contents();
    }

    /* let validation jobs run */
    bt_dm_periodic(a->bt, NULL);
    bt_dm_periodic(b->bt, NULL);

    CuAssertTrue(tc, 1 ==
                 bt_piecedb_all_pieces_are_complete(bt_dm_get_piecedb(b->bt)));
}

