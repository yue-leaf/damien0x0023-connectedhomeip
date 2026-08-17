cmake_minimum_required(VERSION 3.20)

project(lwip)

set(LWIP_ROOT "${bee_lwip_path}")
set(API_DIR "${LWIP_ROOT}/../repo/api")
set(CORE_DIR "${LWIP_ROOT}/../repo/core")
set(NETIF_DIR "${LWIP_ROOT}/../repo/netif")
set(INCLUDE_DIR "${LWIP_ROOT}/../repo/include")
set(ARCH_DIR "${LWIP_ROOT}/arch")

set(lwip lwip)
set(list_lwip_sources lwip_sources)

list(
    APPEND ${list_lwip_sources}

	#LWIP API
	${API_DIR}/api_lib.c
	${API_DIR}/api_msg.c
	${API_DIR}/err.c
	${API_DIR}/if_api.c
	${API_DIR}/netbuf.c
	${API_DIR}/netdb.c
	${API_DIR}/netifapi.c
	${API_DIR}/sockets.c
	${API_DIR}/tcpip.c

	#LWIP CORE
	${CORE_DIR}/altcp.c
	${CORE_DIR}/altcp_alloc.c
	${CORE_DIR}/altcp_tcp.c
	${CORE_DIR}/def.c
	${CORE_DIR}/dns.c
	${CORE_DIR}/inet_chksum.c
	${CORE_DIR}/init.c
	${CORE_DIR}/ip.c
	${CORE_DIR}/mem.c
	${CORE_DIR}/memp.c
	${CORE_DIR}/netif.c
	${CORE_DIR}/pbuf.c
	${CORE_DIR}/raw.c
	${CORE_DIR}/stats.c
	${CORE_DIR}/sys.c
	${CORE_DIR}/tcp.c
	${CORE_DIR}/tcp_in.c
	${CORE_DIR}/tcp_out.c
	${CORE_DIR}/timeouts.c
	${CORE_DIR}/udp.c
	#LWIP CORE IPV4
	${CORE_DIR}/ipv4/autoip.c
	${CORE_DIR}/ipv4/dhcp.c
	${CORE_DIR}/ipv4/etharp.c
	${CORE_DIR}/ipv4/icmp.c
	${CORE_DIR}/ipv4/igmp.c
	${CORE_DIR}/ipv4/ip4.c
	${CORE_DIR}/ipv4/ip4_addr.c
	${CORE_DIR}/ipv4/ip4_frag.c
	#LWIP CORE IPV6
	${CORE_DIR}/ipv6/dhcp6.c
	${CORE_DIR}/ipv6/ethip6.c
	${CORE_DIR}/ipv6/icmp6.c
	${CORE_DIR}/ipv6/inet6.c
	${CORE_DIR}/ipv6/ip6.c
	${CORE_DIR}/ipv6/ip6_addr.c
	${CORE_DIR}/ipv6/ip6_frag.c
	${CORE_DIR}/ipv6/mld6.c
	${CORE_DIR}/ipv6/nd6.c

	#LWIP NETIF
	${NETIF_DIR}/bridgeif.c
	${NETIF_DIR}/bridgeif_fdb.c
	${NETIF_DIR}/ethernet.c
	${NETIF_DIR}/lowpan6.c
	${NETIF_DIR}/lowpan6_ble.c
	${NETIF_DIR}/lowpan6_common.c
	${NETIF_DIR}/zepif.c

	#LWIP ARCH
	${ARCH_DIR}/sys_arch.c
)


add_library(
    ${lwip}
    STATIC
    ${lwip_sources}
)

target_include_directories(
    ${lwip}
    PUBLIC

    ${REALTEK_SDK_INCPATH}
    ${OT_REALTEK_ROOT}/vendor/${RT_PLATFORM}
    ${OT_REALTEK_ROOT}/vendor/${RT_PLATFORM}/${BUILD_TARGET}
    "${ARCH_DIR}"
    "${LWIP_ROOT}"
    "${INCLUDE_DIR}"
    "${INCLUDE_DIR}/lwip"
    "${INCLUDE_DIR}/lwip/apps"
    "${INCLUDE_DIR}/lwip/priv"
    "${INCLUDE_DIR}/lwip/prot"
    "${INCLUDE_DIR}/netif"
    "${INCLUDE_DIR}/netif/ppp"
)

list(
    APPEND openthread_cpp_flags

	-Wno-unused-parameter
	-std=gnu++11
	-std=c++14
	-fno-rtti
)
#target_compile_definitions(${openthread} PRIVATE ${openthread_flags} )
#target_compile_options(${openthread} PRIVATE ${openthread_cpp_flags})

# move static library post build command
#add_custom_command(
#    TARGET ${lwip}
#    POST_BUILD
#    COMMAND cp lib${lwip}.a ${CMAKE_CURRENT_SOURCE_DIR}/.
#)
