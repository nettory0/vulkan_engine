#include "interface_menu.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <format>
#include <chrono>
#include "co_logging.h"


namespace interface_menu
{



CRenderInterface::CRenderInterface()
{}
void CRenderInterface::RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture)
{
	float x_ndc = (2 * translation.x / 1920.);
	float y_ndc = (2 * translation.y / 1080.);

	m_drawQueue.push_back(SUiDrawItem{reinterpret_cast<commons_struct::SIndexedVx*>(geometry), glm::vec2(x_ndc, y_ndc), reinterpret_cast<commons_struct::STexture*>(texture)});
}
void CRenderInterface::EnableScissorRegion(bool enable)
{}
void CRenderInterface::SetScissorRegion(Rml::Rectanglei new_region)
{}

constexpr int kChannels = 4;

void PremultiplyAlpha(std::vector<unsigned char>& data)
{
	for(size_t i = 0; i + 3 < data.size(); i += 4)
	{
		const unsigned int a = data[i + 3];
		data[i + 0] = static_cast<unsigned char>((data[i + 0] * a) / 255);
		data[i + 1] = static_cast<unsigned char>((data[i + 1] * a) / 255);
		data[i + 2] = static_cast<unsigned char>((data[i + 2] * a) / 255);
	}
}

Rml::TextureHandle CRenderInterface::LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source)
{
	Rml::FileInterface* file_interface = Rml::GetFileInterface();
	if(!file_interface)
		return 0;

	Rml::FileHandle file = file_interface->Open(source);
	if(!file)
	{
		Rml::Log::Message(Rml::Log::LT_WARNING, ": %s", source.c_str());
		common::logWarning(source);
		return 0;
	}

	const size_t file_size = file_interface->Length(file);
	std::vector<unsigned char> file_data(file_size);
	const size_t read_size = file_interface->Read(file_data.data(), file_size, file);
	file_interface->Close(file);

	if(file_size == 0 || read_size != file_size)
	{
		Rml::Log::Message(Rml::Log::LT_WARNING, ": %s", source.c_str());
		return 0;
	}

	int width = 0, height = 0, source_channels = 0;
	unsigned char* pixels = stbi_load_from_memory(file_data.data(), static_cast<int>(file_data.size()),  &width, &height, &source_channels, kChannels);
	if(!pixels)
	{
		Rml::Log::Message(Rml::Log::LT_WARNING, "stb_image '%s': %s", source.c_str(), stbi_failure_reason());
		common::logWarning(source);
		return 0;
	}

	commons_struct::STexture* texture = new commons_struct::STexture();
	texture->w = width;
	texture->h = height;
	texture->texChannels = kChannels;
	texture->m_data.assign(pixels, pixels + static_cast<size_t>(width) * height * kChannels);

	stbi_image_free(pixels);

	PremultiplyAlpha(texture->m_data);

	texture_dimensions = Rml::Vector2i(width, height);
	m_texture.insert(texture);
	return reinterpret_cast<Rml::TextureHandle>(texture);
}

Rml::TextureHandle CRenderInterface::GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions)
{
	if(source_dimensions.x <= 0 || source_dimensions.y <= 0)
		return 0;

	const size_t expected_size = static_cast<size_t>(source_dimensions.x) * source_dimensions.y * kChannels;
	if(source.size() != expected_size)
	{
		Rml::Log::Message(Rml::Log::LT_WARNING, "GenerateTexture: %zu,  %zu", expected_size, source.size());
		common::logWarning(std::string("GenerateTexture"));
		return 0;
	}

	commons_struct::STexture* texture = new commons_struct::STexture();
	texture->w = source_dimensions.x;
	texture->h = source_dimensions.y;
	texture->texChannels = kChannels;
	texture->m_data.assign(source.data(), source.data() + source.size());
	m_texture.insert(texture);
	return reinterpret_cast<Rml::TextureHandle>(texture);
}

void CRenderInterface::ReleaseTexture(Rml::TextureHandle texture)
{
	m_texture.erase(reinterpret_cast<commons_struct::STexture*>(texture));
	delete reinterpret_cast<commons_struct::STexture*>(texture);
}

Rml::CompiledGeometryHandle CRenderInterface::CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices)
{
	auto* geom = new commons_struct::SIndexedVx();
	geom->m_aVx.reserve(vertices.size());
	geom->m_aIndices.reserve(indices.size());
	for(const auto& point : vertices)
	{
		
		commons_struct::SVertex vertex;
		float x_ndc = (2. * point.position.x / 1920.) - 1;
		float y_ndc = (2. * point.position.y / 1080.) - 1;
		vertex.pos = glm::vec4(x_ndc, y_ndc, 0, 1);
		vertex.texCoord = glm::vec2(point.tex_coord.x, point.tex_coord.y);
		vertex.color = glm::vec4((float)point.colour.red/255., (float)point.colour.green / 255., (float)point.colour.blue / 255., (float)point.colour.alpha / 255.);
		geom->m_aVx.push_back(vertex);
	}

	for(const auto& index : indices)
		geom->m_aIndices.push_back(index);
	m_geometry.insert(geom);
	return reinterpret_cast<Rml::CompiledGeometryHandle>(geom);
}

void CRenderInterface::ReleaseGeometry(Rml::CompiledGeometryHandle geometry)
{
	m_geometry.erase(reinterpret_cast<commons_struct::SIndexedVx*>(geometry));
	delete reinterpret_cast<commons_struct::SIndexedVx*>(geometry);
}

const std::vector<SUiDrawItem>& CRenderInterface::getDrawQueue() const
{
	return m_drawQueue;
}

void CRenderInterface::clearDrawQueue()
{
	m_drawQueue = {};
}

const std::set<commons_struct::STexture*>& CRenderInterface::avalibelTextures() const
{
	return m_texture;
}

const std::set<commons_struct::SIndexedVx*>& CRenderInterface::avalibelGeometry() const
{
	return m_geometry;
}

CInterfaceManager::CInterfaceManager():m_interfaceProvider{std::make_unique<CRenderInterface>()}, m_systemInterfaceProvider{std::make_unique<CSystemInterface>()}
{}

CInterfaceManager::~CInterfaceManager()
{
	Rml::Shutdown();
}

bool CInterfaceManager::init()
{
	Rml::SetRenderInterface(m_interfaceProvider.get());
	Rml::SetSystemInterface(m_systemInterfaceProvider.get());

	if(!Rml::Initialise())
		return false;
	if(m_pContext = Rml::CreateContext("menu", {1920, 1080}); m_pContext == nullptr)
		return false;

	Rml::LoadFontFace("models/fonts/LiberationSans-Regular.ttf");
	loadInterface("models/ui/demo.rml");
	return true;
}

bool CInterfaceManager::loadInterface(std::string path)
{
	if(!m_pContext)
		return false;

	auto* doc = m_pContext->LoadDocument(path);
	if(!doc)
	{
		common::logError(std::string("Document load failed\n"));
		return false;
	}
	doc->Show();
	m_fps = doc->GetElementById("fps");

	return true;
}

void CInterfaceManager::destroy()
{
	m_pContext = nullptr;
	Rml::RemoveContext("menu");
	Rml::Shutdown();
}

static int counter{0};

void CInterfaceManager::render()
{

	m_interfaceProvider->clearDrawQueue();
	if(m_pContext)
		m_pContext->Render();
}

void CInterfaceManager::update()
{
	using clock = std::chrono::steady_clock;
	static auto lastTime = clock::now();
	static int frames = 0;
	static int fps = 0;

	++frames;
	const auto now = clock::now();
	const double elapsed = std::chrono::duration<double>(now - lastTime).count();
	if(elapsed >= 1.0)
	{
		fps = static_cast<int>(frames / elapsed);
		frames = 0;
		lastTime = now;

		if(m_fps)
			m_fps->SetInnerRML("FPS: " + std::to_string(fps));
		common::logInfo(std::format("fps {}", fps));
	}

	if(m_pContext)
		m_pContext->Update();
}

const std::vector<SUiDrawItem>& CInterfaceManager::getDrawQueue() const
{
	return m_interfaceProvider->getDrawQueue();
}

const std::set<commons_struct::STexture*>& CInterfaceManager::avalibelTextures() const
{
	return m_interfaceProvider->avalibelTextures();
}

const std::set<commons_struct::SIndexedVx*>& CInterfaceManager::avalibelGeometry() const
{
	return m_interfaceProvider->avalibelGeometry();
}

double CSystemInterface::GetElapsedTime()
{
	return 0.0;
}

bool CSystemInterface::LogMessage(Rml::Log::Type type, const Rml::String& message)
{
	std::string comment{"[RmlUi] "};
	comment.append(std::string(message));
	common::logError(message);
	return true;
}

};