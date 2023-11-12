#pragma once

#include "Walnut/Core/Buffer.h"

#include <string>
#include <map>
#include <thread>
#include <functional>

// Forward declarations for GNS API
class ISteamNetworkingSockets;
struct SteamNetConnectionStatusChangedCallback_t;

namespace Walnut {

	// HSteamNetConnection in GNS API
	using ClientID = uint32_t;

	struct ClientInfo
	{
		ClientID ID;
		std::string ConnectionDesc;
	};

	class Server
	{
	public:
		using DataReceivedCallback = std::function<void(const ClientInfo&, const Buffer)>;
		using ClientConnectedCallback = std::function<void(const ClientInfo&)>;
		using ClientDisconnectedCallback = std::function<void(const ClientInfo&)>;
	public:
		Server(int port);
		~Server();

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Start and Stop the server
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		void Start();
		void Stop();

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Set callbacks for server events
		// These callbacks will be called from the server thread
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		void SetDataReceivedCallback(const DataReceivedCallback& function);
		void SetClientConnectedCallback(const ClientConnectedCallback& function);
		void SetClientDisconnectedCallback(const ClientDisconnectedCallback& function);

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Send Data
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		void SendBufferToClient(ClientID clientID, Buffer buffer, bool reliable = true);
		void SendBufferToAllClients(Buffer buffer, ClientID excludeClientID = 0, bool reliable = true);

		void SendStringToClient(ClientID clientID, const std::string& string, bool reliable = true);
		void SendStringToAllClients(const std::string& string, ClientID excludeClientID = 0, bool reliable = true);

		template<typename T>
		void SendDataToClient(ClientID clientID, const T& data, bool reliable = true)
		{
			SendBufferToClient(clientID, Buffer(&data, sizeof(T)), reliable);
		}

		template<typename T>
		void SendDataToAllClients(const T& data, ClientID excludeClientID = 0, bool reliable = true)
		{
			SendBufferToAllClients(Buffer(&data, sizeof(T)), excludeClientID, reliable);
		}
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		void KickClient(ClientID clientID);

		bool IsRunning() const { return m_Running; }
		const std::map<ClientID, ClientInfo>& GetConnectedClients() const { return m_ConnectedClients; }
	private:
		void NetworkThreadFunc(); // Server thread

		static void ConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* info);
		void OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info);

		// Server functionality
		void PollIncomingMessages();
		void SetClientNick(ClientID hConn, const char* nick);
		void PollConnectionStateChanges();

		void OnFatalError(const std::string& message);
	private:
		std::thread m_NetworkThread;
		DataReceivedCallback m_DataReceivedCallback;
		ClientConnectedCallback m_ClientConnectedCallback;
		ClientDisconnectedCallback m_ClientDisconnectedCallback;

		int m_Port = 0;
		bool m_Running = false;
		std::map<ClientID, ClientInfo> m_ConnectedClients;

		ISteamNetworkingSockets* m_Interface = nullptr;
		uint32_t m_ListenSocket = 0u;

		// HSteamNetPollGroup
		uint32_t m_PollGroup = 0u;
	};

}