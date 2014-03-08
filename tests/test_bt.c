
#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "CuTest.h"

#include <stdint.h>

#include "bt.h"

/*
 * bt_sha1_equal tells if a sha1 hash is equal or not
 */
void TestBT_Sha1Equal(
    CuTest * tc
)
{
    char *s1 = "00000000000000000000";

    char *s2 = "00000000000000000000";

    char *s3 = "10000000000000000000";

    CuAssertTrue(tc, 1 == bt_sha1_equal(s1, s2));
    CuAssertTrue(tc, 0 == bt_sha1_equal(s1, s3));
}

void TestBT_GeneratedPeeridIs20BytesLong(
    CuTest * tc
)
{
    char *peerid;

    peerid = bt_generate_peer_id();

    CuAssertTrue(tc, 20 == strlen(peerid));
}
