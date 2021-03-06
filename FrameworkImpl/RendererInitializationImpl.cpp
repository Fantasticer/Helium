#include "FrameworkImplPch.h"
#include "FrameworkImpl/RendererInitializationImpl.h"
#include "Windowing/WindowManager.h"
#include "Engine/Config.h"
#include "Graphics/GraphicsConfig.h"

#if HELIUM_DIRECT3D
# include "RenderingD3D9/D3D9Renderer.h"
#else
# include "RenderingGL/GLRenderer.h"
#endif

#include "Graphics/RenderResourceManager.h"
#include "Graphics/DynamicDrawer.h"

using namespace Helium;

/// @copydoc RendererInitialization::Initialize()
bool RendererInitializationImpl::Initialize()
{
	WindowManager* pWindowManager = WindowManager::GetStaticInstance();
	if( !pWindowManager )
	{
		HELIUM_TRACE(
			TraceLevels::Info,
			( TXT( "RendererInitializationImpl::Initialize(): No window manager created.  A window manager is necessary for " )
			TXT( "RendererInitializationImpl execution.\n" ) ) );

		return false;
	}

	Renderer* pRenderer = NULL;

#if HELIUM_DIRECT3D
	if( !D3D9Renderer::CreateStaticInstance() )
	{
		return false;
	}
	pRenderer = D3D9Renderer::GetStaticInstance();
#elif HELIUM_OPENGL
	if( !GLRenderer::CreateStaticInstance() )
	{
		return false;
	}
	pRenderer = GLRenderer::GetStaticInstance();
#endif
	HELIUM_ASSERT( pRenderer );
	if( !pRenderer->Initialize() )
	{
		Renderer::DestroyStaticInstance();
		return false;
	}

	// Create the main application window.
	Config& rConfig = Config::GetStaticInstance();
	StrongPtr< GraphicsConfig > spGraphicsConfig(
		rConfig.GetConfigObject< GraphicsConfig >( Name( "GraphicsConfig" ) ) );
	HELIUM_ASSERT( spGraphicsConfig );

	uint32_t displayWidth = spGraphicsConfig->GetWidth();
	uint32_t displayHeight = spGraphicsConfig->GetHeight();
	bool bFullscreen = spGraphicsConfig->GetFullscreen();
	bool bVsync = spGraphicsConfig->GetVsync();

	Window::Parameters windowParameters;
	windowParameters.pTitle = "Helium";
	windowParameters.width = displayWidth;
	windowParameters.height = displayHeight;
	windowParameters.bFullscreen = bFullscreen;

	m_pMainWindow = pWindowManager->Create( windowParameters );
	HELIUM_ASSERT( m_pMainWindow );
	if( !m_pMainWindow )
	{
		HELIUM_TRACE( TraceLevels::Error, TXT( "Failed to create main application window.\n" ) );

		return false;
	}

	m_pMainWindow->SetOnDestroyed( Delegate<Window*>( this, &RendererInitializationImpl::OnMainWindowDestroyed ) );

	// Create the application rendering context.
	Renderer::ContextInitParameters contextInitParams;
	contextInitParams.pWindow = m_pMainWindow->GetHandle();
	contextInitParams.displayWidth = displayWidth;
	contextInitParams.displayHeight = displayHeight;
	contextInitParams.bFullscreen = bFullscreen;
	contextInitParams.bVsync = bVsync;

	bool bContextCreateResult = pRenderer->CreateMainContext( contextInitParams );
	HELIUM_ASSERT( bContextCreateResult );
	if( !bContextCreateResult )
	{
		HELIUM_TRACE( TraceLevels::Error, TXT( "Failed to create main renderer context.\n" ) );

		return false;
	}

	// Create and initialize the render resource manager.
	RenderResourceManager& rRenderResourceManager = RenderResourceManager::GetStaticInstance();
	rRenderResourceManager.Initialize();

	// Create and initialize the dynamic drawing interface.
	DynamicDrawer& rDynamicDrawer = DynamicDrawer::GetStaticInstance();
	if( !rDynamicDrawer.Initialize() )
	{
		HELIUM_TRACE( TraceLevels::Error, "Failed to initialize dynamic drawing support.\n" );
		return false;
	}
	return true;
}

/// Callback executed when the main window is actually destroyed.
///
/// @param[in] pWindow  Pointer to the destroyed Window instance.
void RendererInitializationImpl::OnMainWindowDestroyed( Window* pWindow )
{
	HELIUM_ASSERT( m_pMainWindow == pWindow );
	HELIUM_UNREF( pWindow );

#if HELIUM_OPENGL
	// Immediately shut down, since we use GLFW to manage windows, and GLFW
	// windows are inseparable from their render contexts.  Therefore, by the
	// time we've received this callback, our renderer had better be shutting down.
	Renderer* pRenderer = Renderer::GetStaticInstance();
	pRenderer->Shutdown();
#endif

	m_pMainWindow = NULL;
	WindowManager* pWindowManager = WindowManager::GetStaticInstance();
	HELIUM_ASSERT( pWindowManager );
	pWindowManager->RequestQuit();
}

void Helium::RendererInitializationImpl::Shutdown()
{
	DynamicDrawer::DestroyStaticInstance();
	RenderResourceManager::DestroyStaticInstance();

	Renderer* pRenderer = Renderer::GetStaticInstance();
	if( pRenderer )
	{
		pRenderer->Shutdown();
		Renderer::DestroyStaticInstance();
	}

	WindowManager* pWindowManager = WindowManager::GetStaticInstance();
	if( pWindowManager )
	{
		if( m_pMainWindow )
		{
			m_pMainWindow->Destroy();
			while( m_pMainWindow )
			{
				pWindowManager->Update();
			}
		}

		pWindowManager->Shutdown();
		WindowManager::DestroyStaticInstance();
	}
}