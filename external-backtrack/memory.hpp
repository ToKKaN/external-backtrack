#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <memory>
#include <string>
#include <unordered_map>

class c_module
{
public:
	c_module( const ptrdiff_t  img_base, const ptrdiff_t  img_size, const std::string& img_name, const std::string& img_path ) :
		image_size_( img_size ),
		image_base_( img_base ),
		image_name_( img_name ),
		image_path_( img_path )
	{
		
	}
	const std::string&	get_path() const
	{
		return image_path_;
	}
	ptrdiff_t  get_image_base() const
	{
		return image_base_;
	}
	const std::string&	get_name() const
	{
		return image_name_;
	}
	ptrdiff_t  get_size() const
	{
		return image_size_;
	}
	void reset()
	{
		image_size_ = NULL;
		image_name_.clear();
		image_path_.clear();
		image_base_ = NULL;
	}

private:
	uintptr_t 					image_size_;
	ptrdiff_t 					image_base_;
	std::string					image_name_;
	std::string					image_path_;
};

class c_memory
{
public:
	bool read( const ptrdiff_t  dw_address, const LPVOID& buffer, const uintptr_t size ) const
	{
		SIZE_T out = NULL;

		return ( ReadProcessMemory( process_handle_, reinterpret_cast< LPVOID >( dw_address ), buffer, size, &out ) == TRUE );
	}
	bool write( const ptrdiff_t  dw_address, const LPCVOID buffer, const uintptr_t size ) const
	{
		SIZE_T out = NULL;

		return ( WriteProcessMemory( process_handle_, reinterpret_cast< LPVOID >( dw_address ), buffer, size, &out ) == TRUE );
	}
	template<typename T>
	T read_memory( const ptrdiff_t  address, const T& default_ret = T() )
	{
		T t_return;

		if( !read( address, &t_return, sizeof( T ) ) )
			return default_ret;

		return t_return;
	}
	template<typename T>
	bool write_memory( const ptrdiff_t  address, const T& value )
	{
		return write( address, &value, sizeof( T ) );
	}
	template<typename T>
	bool write_memory_protected( const ptrdiff_t  address, const T& value )
	{
		DWORD_PTR  old;
		VirtualProtectEx( process_handle_, reinterpret_cast< LPVOID >( address ), sizeof( T ), PAGE_EXECUTE_READWRITE, &old );
		if( !write_memory<T>( address, value ) )
		{
			VirtualProtectEx( process_handle_, reinterpret_cast< LPVOID >( address ), sizeof( T ), old, nullptr );
			return false;
		}
		VirtualProtectEx( process_handle_, reinterpret_cast< LPVOID >( address ), sizeof( T ), old, nullptr );

		return true;
	}
	bool attach_process( const std::string& name )
	{
		detach_process();

		if( name.empty() ) // no process given
			return false;

		process_id_ = get_process_id( name );

		if( !process_id_ ) // process not running
			return false;

		process_handle_ = OpenProcess( PROCESS_ALL_ACCESS, false, process_id_ );

		if( !process_handle_ ) // cant open process
			return false;

		if( !get_all_modules() ) // could not find any modules
			return false;

		return true; // everything went smooth
	}
	void detach_process()
	{
		if( process_handle_ )
			CloseHandle( process_handle_ );

		process_handle_ = nullptr;
		process_id_ = NULL;
		process_hwnd_ = nullptr;

		modules_.clear();
	}
	static DWORD get_process_id( const std::string& name )
	{
		const auto h_snapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, NULL );

		if( h_snapshot == INVALID_HANDLE_VALUE )
			return NULL;

		PROCESSENTRY32 entry;

		entry.dwSize = sizeof( PROCESSENTRY32 );

		if( !Process32First( h_snapshot, &entry ) )
		{
			CloseHandle( h_snapshot );
			return NULL;
		}

		do
		{
			if( name == entry.szExeFile )
				break;
		} while( Process32Next( h_snapshot, &entry ) );


		CloseHandle( h_snapshot );

		return entry.th32ProcessID;
	}
	c_module* get_module( const std::string& name )
	{
		if( modules_.empty() ) // no modules loaded, probably not attached to any process yet
			return nullptr;

		for( const auto& m : modules_ )
		{
			if( m.first == name )
				return m.second; // module found
		}

		return nullptr; // module not found
	}
	HMODULE load_remote_module( const std::string& name )
	{
		if( modules_.empty() ) // no modules loaded, probably not attached to any process yet
			return nullptr;

		for( const auto& m : modules_ )
		{
			if( m.first == name )
				return LoadLibraryA( m.second->get_path().c_str() );
		}

		return nullptr; // module not found
	}
	void set_process_window( const HWND window )
	{
		process_hwnd_ = window;
	}
	HWND get_window() const
	{
		return process_hwnd_;
	}
private:
	bool get_all_modules()
	{
		const auto h_snapshot = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, process_id_ );

		if( h_snapshot == INVALID_HANDLE_VALUE )
			return false;

		MODULEENTRY32 entry;

		entry.dwSize = sizeof( MODULEENTRY32 );

		if( !Module32First( h_snapshot, &entry ) )
		{
			CloseHandle( h_snapshot );
			return false;
		}

		c_module* ptr_module = nullptr;

		do
		{
			char path[ MAX_PATH ];

			GetModuleFileNameExA( process_handle_, entry.hModule, path, MAX_PATH );

			ptr_module = new c_module( reinterpret_cast< ptrdiff_t >( entry.hModule ), entry.modBaseSize, entry.szModule, path );
			modules_.insert_or_assign( entry.szModule, ptr_module );
		} while( Module32Next( h_snapshot, &entry ) );

		CloseHandle( h_snapshot );

		return !modules_.empty();
	}
	DWORD												process_id_ = NULL;
	HANDLE												process_handle_ = nullptr;
	HWND												process_hwnd_ = nullptr;
	std::unordered_map< std::string, c_module* >		modules_;
	std::unordered_map< uintptr_t, std::string >		tables_;
};

extern std::unique_ptr<c_memory> g_ptr_memory;