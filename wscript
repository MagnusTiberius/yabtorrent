# -*- mode: python -*-
# vi: set ft=python :

import sys
import os

def options(opt):
        opt.load('compiler_c')

def configure(conf):
    
    conf.load('compiler_c')
    conf.load('clib')

    if sys.platform == 'win32':
        conf.check_cc(lib='ws2_32')
        conf.check_cc(lib='psapi')

    conf.check_cc(lib='uv', libpath=[os.getcwd()])

def unit_test(bld, src, ccflag=None):
    target = "build/tests/t_{0}".format(src)

    # collect tests into one area
    bld(rule='sh {0}/deps/cutest/make-tests.sh {0}/tests/{1} > {2}'.format(os.getcwd(), src, target), target=target)

    libs = []

    # build the test program
    bld.program(
        source=[
            "tests/{0}".format(src),
            target,
            ] + bld.clib_c_files("""
                bitfield
                cutest
                """.split()),
        target=src[:-2],
        cflags=[
            '-g',
            '-Werror',
        ],
        use='yabbt',
        lib = libs,
        unit_test='yes',
        includes=[ "./include" ] + bld.clib_h_paths("""
                                    asprintf
                                    cutest
                                    """.split())
        )

    # run the test
    if sys.platform == 'win32':
        bld(rule='${SRC}',source=src[:-2]+'.exe')
    else:
        bld(rule='export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:. && ./${SRC}', source=src[:-2])

def scenario_test(bld, src, ccflag=None):
    src = "tests/"+src
    #bld(rule='cp ../tests/make-tests.sh .')
    #bld(rule='echo $(pwd)')
    #bld(rule='echo $(pwd) && cp %s .' % src)
    # collect tests into one area
    bld(rule='sh ../make-tests.sh ../'+src+' > ${TGT}', target=src+".autogenerated.c")

    libs = []
    bld.program(
        source=[
            src,
            src+".autogenerated.c",
            "tests/CuTest.c",
            "src/network_adapter_mock.c",
            "tests/mock_torrent.c",
            "tests/mock_client.c",
            #bld.env.CONTRIB_PATH+"PeerWireProtocol/pwp_handshaker.c"
            ] +\
            bld.clib_c_files([
                'mt19937ar',
                'pwp']),
        stlibpath = ['libuv','.'],
        target=src[:-2],
        cflags=[
            '-g',
            '-Werror',
            '-Werror=unused-variable',
            '-Werror=uninitialized',
            '-Werror=return-type',
            ],
        lib = libs,
        unit_test='yes',
        includes=[
            "./include",
            "./tests",
            bld.env.CONTRIB_PATH+"CBitfield",
            bld.env.CONTRIB_PATH+"CSimpleBitstream",
            bld.env.CONTRIB_PATH+"PeerWireProtocol",
            bld.env.CONTRIB_PATH+"CConfig-re",
            bld.env.CONTRIB_PATH+"CBTTrackerClient",
            bld.env.CONTRIB_PATH+"CHeaplessBencodeReader",
            bld.env.CONTRIB_PATH+"CTorrentFileReader",
            bld.env.CONTRIB_PATH+"CHashMapViaLinkedList",
            bld.env.CONTRIB_PATH+"CBipBuffer",
           ], 
        use='yabbt')

    # run the test
    if sys.platform == 'win32':
        bld(rule='${SRC}',source=src[:-2]+'.exe')
    else:
        bld(rule='export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:. && ./${SRC}',source=src[:-2])
        #bld(rule='pwd && export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:. && ./'+src[:-2])

def build(bld):
    bld.load('clib')

    # Copy libuv.a to build/
    #bld(rule='cp '+cp+'/libuv/.libs/libuv.a .', always=True)#, target="libuv.a")

    if sys.platform == 'win32':
        platform = '-DWIN32'
    elif sys.platform == 'linux2':
        platform = '-DLINUX'
    else:
        platform = ''

    libyabtorrent_clibs = """
        array-avl-tree
        asprintf
        bag
        bitfield
        bitstream
        chunkybar
        config-re
        event-timer
        file2str
        heap
        heapless-bencode
        linked-list-hashmap
        linked-list-queue
        meanqueue
        pwp
        pseudolru
        sha1
        strndup
        stubfile
        """.split()

    bld.shlib(
        source="""
        src/bt_blacklist.c
        src/bt_choker_leecher.c
        src/bt_choker_seeder.c
        src/bt_diskcache.c
        src/bt_diskmem.c
        src/bt_download_manager.c
        src/bt_filedumper.c
        src/bt_peer_manager.c
        src/bt_piece.c
        src/bt_piece_db.c
        src/bt_selector_random.c
        src/bt_selector_rarestfirst.c
        src/bt_selector_sequential.c
        src/bt_util.c
        src/bt_sha1.c""".split() + bld.clib_c_files(libyabtorrent_clibs),
        includes=['./include'] + bld.clib_h_paths(libyabtorrent_clibs),
        target='yabbt',
        #lib=libs,
        cflags=[
            '-Werror',
            '-Werror=format',
            '-Werror=int-to-pointer-cast',
            '-g',
            platform,
            '-Werror=unused-variable',
            '-Werror=return-type',
            '-Werror=uninitialized',
            '-Werror=pointer-to-int-cast',
            '-Wcast-align'])

    unit_test(bld,"test_bt.c")
    #unit_test(bld,"test_download_manager.c")
    #unit_test(bld,"test_peer_manager.c")
    #unit_test(bld,'test_choker_leecher.c')
    #unit_test(bld,'test_choker_seeder.c')
    #unit_test(bld,'test_selector_rarestfirst.c')
    #unit_test(bld,'test_selector_random.c')
    #unit_test(bld,'test_selector_sequential.c')
    #unit_test(bld,'test_piece.c',ccflag='-I../'+cp+"CBitfield")
    #unit_test(bld,'test_piece_db.c')
    #unit_test(bld,'test_blacklist.c')
    #scenario_test(bld,'test_scenario_shares_all_pieces.c')
    #scenario_test(bld,'test_scenario_shares_all_pieces_between_each_other.c')
    #scenario_test(bld,'test_scenario_share_20_pieces.c')
    #scenario_test(bld,'test_scenario_three_peers_share_all_pieces_between_each_other.c')

    libs = ['yabbt', 'uv']
    if sys.platform == 'win32':
        libs += """
                ws2_32
                psapi
                Iphlpapi
                """.split()
    elif sys.platform == 'darwin':
        pass
    else:
        libs += """
                dl
                rt
                pthread
                """.split()


    bld.program(
        source="""
            src/yabtorrent.c
            src/network_adapter_libuv_v0.10.c
            """.split() + bld.clib_c_files("""
                mt19937ar
                tracker-client
                torrent-reader
                """.split()),
        target='yabtorrent',
        cflags="""
            -g
            -Werror
            -Werror=uninitialized
            -Werror=pointer-to-int-cast
            -Werror=return-type
            """.split(),
        stlibpath=['.'],
	libpath=[os.getcwd()],
        lib=libs,
        includes=['./include', './libuv/include'] + bld.clib_h_paths("""
                asprintf
                config-re
                file2str
                heapless-bencode
                linked-list-hashmap
                linked-list-queue
                pwp
                strndup
                tracker-client
                torrent-reader
                """.split()))
