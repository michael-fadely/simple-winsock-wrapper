#pragma once
#include <winerror.h>

namespace sws
{
	/**
	 * \brief Native Winsock error codes in enum form.
	 */
	enum class SocketError : int
	{
		none                         = 0,
		access                       = WSAEACCES,
		addr_in_use                  = WSAEADDRINUSE,
		addr_not_available           = WSAEADDRNOTAVAIL,
		unsupported_address_family   = WSAEAFNOSUPPORT,
		already_in_progress          = WSAEALREADY,
		bad_file_handle              = WSAEBADF,
		cancelled                    = WSAECANCELLED,
		connection_aborted           = WSAECONNABORTED,
		connection_refused           = WSAECONNREFUSED,
		connection_reset             = WSAECONNRESET,
		destination_address_required = WSAEDESTADDRREQ,
		disconnected                 = WSAEDISCON,
		disk_quota                   = WSAEDQUOT,
		fault                        = WSAEFAULT,
		host_down                    = WSAEHOSTDOWN,
		host_unreachable             = WSAEHOSTUNREACH,
		in_progress                  = WSAEINPROGRESS,
		interrupted                  = WSAEINTR,
		invalid                      = WSAEINVAL,
		invalid_proc_table           = WSAEINVALIDPROCTABLE,
		invalid_provider             = WSAEINVALIDPROVIDER,
		is_connected                 = WSAEISCONN,
		loop                         = WSAELOOP,
		too_many_sockets             = WSAEMFILE,
		message_too_large            = WSAEMSGSIZE,
		name_too_long                = WSAENAMETOOLONG,
		network_down                 = WSAENETDOWN,
		network_reset                = WSAENETRESET,
		network_unreachable          = WSAENETUNREACH,
		no_buffers                   = WSAENOBUFS,
		no_more                      = WSAENOMORE,
		invalid_option               = WSAENOPROTOOPT,
		not_connected                = WSAENOTCONN,
		not_empty                    = WSAENOTEMPTY,
		not_socket                   = WSAENOTSOCK,
		unsupported_operation        = WSAEOPNOTSUPP,
		unsupported_protocol_family  = WSAEPFNOSUPPORT,
		proclim                      = WSAEPROCLIM,
		unsupported_protocol         = WSAEPROTONOSUPPORT,
		prototype                    = WSAEPROTOTYPE,
		provider_failed_init         = WSAEPROVIDERFAILEDINIT,
		refused                      = WSAEREFUSED,
		remote                       = WSAEREMOTE,
		shutdown                     = WSAESHUTDOWN,
		unsupported_socket_type      = WSAESOCKTNOSUPPORT,
		stale                        = WSAESTALE,
		timed_out                    = WSAETIMEDOUT,
		too_many_references          = WSAETOOMANYREFS,
		users                        = WSAEUSERS,
		would_block                  = WSAEWOULDBLOCK,
		host_not_found               = WSAHOST_NOT_FOUND,
		not_initialized              = WSANOTINITIALISED,
		no_data                      = WSANO_DATA,
		no_recovery                  = WSANO_RECOVERY,
		service_not_found            = WSASERVICE_NOT_FOUND,
		syscall_failure              = WSASYSCALLFAILURE,
		sys_not_ready                = WSASYSNOTREADY,
		try_again                    = WSATRY_AGAIN,
		type_not_found               = WSATYPE_NOT_FOUND,
		unsupported_version          = WSAVERNOTSUPPORTED,
		e_cancelled                  = WSA_E_CANCELLED,
		e_no_more                    = WSA_E_NO_MORE,
		qos_admission_failure        = WSA_QOS_ADMISSION_FAILURE,
		qos_bad_object               = WSA_QOS_BAD_OBJECT,
		qos_bad_style                = WSA_QOS_BAD_STYLE,
		qos_efiltercount             = WSA_QOS_EFILTERCOUNT,
		qos_efilterstyle             = WSA_QOS_EFILTERSTYLE,
		qos_efiltertype              = WSA_QOS_EFILTERTYPE,
		qos_eflowcount               = WSA_QOS_EFLOWCOUNT,
		qos_eflowdesc                = WSA_QOS_EFLOWDESC,
		qos_eflowspec                = WSA_QOS_EFLOWSPEC,
		qos_eobjlength               = WSA_QOS_EOBJLENGTH,
		qos_epolicyobj               = WSA_QOS_EPOLICYOBJ,
		qos_eprovspecbuf             = WSA_QOS_EPROVSPECBUF,
		qos_epsfilterspec            = WSA_QOS_EPSFILTERSPEC,
		qos_epsflowspec              = WSA_QOS_EPSFLOWSPEC,
		qos_esdmodeobj               = WSA_QOS_ESDMODEOBJ,
		qos_eservicetype             = WSA_QOS_ESERVICETYPE,
		qos_eshaperateobj            = WSA_QOS_ESHAPERATEOBJ,
		qos_eunkownpsobj             = WSA_QOS_EUNKOWNPSOBJ,
		qos_generic_error            = WSA_QOS_GENERIC_ERROR,
		qos_no_receivers             = WSA_QOS_NO_RECEIVERS,
		qos_no_senders               = WSA_QOS_NO_SENDERS,
		qos_policy_failure           = WSA_QOS_POLICY_FAILURE,
		qos_receivers                = WSA_QOS_RECEIVERS,
		qos_request_confirmed        = WSA_QOS_REQUEST_CONFIRMED,
		qos_reserved_petype          = WSA_QOS_RESERVED_PETYPE,
		qos_senders                  = WSA_QOS_SENDERS,
		qos_traffic_ctrl_error       = WSA_QOS_TRAFFIC_CTRL_ERROR,
	};

	/**
	 * \brief Socket state for simple error checking.
	 */
	enum class SocketState
	{
		done,
		in_progress,
		closed,
		error
	};

	/**
	 * \brief Derives a \c sws::SocketState from a \c sws::SocketError
	 * \param error The error to convert.
	 * \return The state derived from the error.
	 */
	inline SocketState to_state(SocketError error)
	{
		switch (error)
		{
			case SocketError::would_block:
			case SocketError::already_in_progress:
				return SocketState::in_progress;

			case SocketError::connection_aborted:
			case SocketError::connection_reset:
			case SocketError::timed_out:
			case SocketError::network_reset:
			case SocketError::not_connected:
				return SocketState::closed;

			// with Winsock, WSAEISCONN may be returned when a
			// non-blocking socket's connection has completed.
			case SocketError::is_connected:
			case SocketError::none: 
				return SocketState::done;

			default:
				return SocketState::error;
		}
	}
}
