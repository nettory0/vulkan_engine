#pragma once

namespace interface_menu
{

struct SUiDrawItem
{
    commons_struct::SIndexedVx* m_pGeometry{nullptr};
    glm::vec2 m_translation;
    commons_struct::STexture* m_pTexture{nullptr};
};
}