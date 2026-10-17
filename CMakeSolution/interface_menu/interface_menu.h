#pragma once
#include <set>
#include "interface_menu_export.h"
#include "co_structs.h"
#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>
#include "interface_types.h"


namespace interface_menu
{

class CRenderInterface: public Rml::RenderInterface
{
public:
    CRenderInterface();
    void RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture) override;
    void EnableScissorRegion(bool enable) override;
    void SetScissorRegion(Rml::Rectanglei new_region) override;
    Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) override;
    Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions) override;
    void ReleaseTexture(Rml::TextureHandle texture) override;
    Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) override;
	void ReleaseGeometry(Rml::CompiledGeometryHandle geometry)  override;
    const std::vector<SUiDrawItem>& getDrawQueue() const;
    void clearDrawQueue();
    const std::set<commons_struct::STexture*>& avalibelTextures() const;
    const std::set<commons_struct::SIndexedVx*>& avalibelGeometry() const;

private:
    std::vector<SUiDrawItem> m_drawQueue;
    std::set<commons_struct::STexture*> m_texture;
    std::set<commons_struct::SIndexedVx*> m_geometry;
};

class CSystemInterface: public Rml::SystemInterface
{
public:
    CSystemInterface() = default;
    double GetElapsedTime() override;
    bool LogMessage(Rml::Log::Type type, const Rml::String& message) override;
};


class CInterfaceManager
{
public:
    CInterfaceManager();
    bool init();
    bool loadInterface(std::string path);
    void destroy();
    void render();
    void update();
    const std::vector<SUiDrawItem>& getDrawQueue() const;
    const std::set<commons_struct::STexture*>& avalibelTextures() const;
    const std::set<commons_struct::SIndexedVx*>& avalibelGeometry() const;
private:
    std::unique_ptr<CRenderInterface> m_interfaceProvider;
    std::unique_ptr<CSystemInterface> m_systemInterfaceProvider;
    Rml::Context* m_pContext{nullptr};
    Rml::Element* m_fps{nullptr};

};

};