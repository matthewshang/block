#include "frustum.h"

void Frustum::setInternals(float angle, float ratio, float nearD, float farD)
{
    const double ang2rad = 3.14159265358979323846 / 180.0;

    m_ratio = ratio;
    m_angle = angle;
    m_nearD = nearD;
    m_farD = farD;

    m_tanG = static_cast<float>(tan(angle * ang2rad * 0.5));
    m_nh = nearD * m_tanG;
    m_nw = m_nh * ratio;
    m_fh = farD * m_tanG;
    m_fw = m_fh * ratio;
}

void Frustum::setCam(const glm::vec3 &p, const glm::vec3 &l, const glm::vec3 &u)
{
    glm::vec3 dir, nc, fc, x, y, z;

    z = glm::normalize(p - l);
    x = glm::normalize(glm::cross(u, z));
    y = glm::cross(z, x);

    nc = p - z * m_nearD;
    fc = p - z * m_farD;

    m_ntl = nc + y * m_nh - x * m_nw;
    m_ntr = nc + y * m_nh + x * m_nw;
    m_nbl = nc - y * m_nh - x * m_nw;
    m_nbr = nc - y * m_nh + x * m_nw;

    m_ftl = fc + y * m_fh - x * m_fw;
    m_ftr = fc + y * m_fh + x * m_fw;
    m_fbl = fc - y * m_fh - x * m_fw;
    m_fbr = fc - y * m_fh + x * m_fw;

    m_p[0].setPoints(m_ntr, m_ntl, m_ftl);
    m_p[1].setPoints(m_nbl, m_nbr, m_fbr);
    m_p[2].setPoints(m_ntl, m_nbl, m_fbl);
    m_p[3].setPoints(m_nbr, m_ntr, m_fbr);
    m_p[4].setPoints(m_ntl, m_ntr, m_nbr);
    m_p[5].setPoints(m_ftr, m_ftl, m_fbl);
}

bool Frustum::boxInFrustum(glm::vec3 &a, glm::vec3 &w)
{
    for (int i = 0; i < 6; i++)
    {
        if (m_p[i].distance(getMaxVert(a, w, m_p[i].getNormal())) < 0)
            return false;
    }

    return true;
}

glm::vec3 Frustum::getMaxVert(const glm::vec3 &corner, const glm::vec3 &size, const glm::vec3 &norm)
{
    glm::vec3 ret = corner;

    if (norm.x > 0)
        ret.x += size.x;

    if (norm.y > 0)
        ret.y += size.y;

    if (norm.z > 0)
        ret.z += size.z;

    return ret;
}

glm::vec3 Frustum::getMinVert(const glm::vec3 &corner, const glm::vec3 &size, const glm::vec3 &norm)
{
    glm::vec3 ret = corner;

    if (norm.x < 0)
        ret.x += size.x;

    if (norm.y < 0)
        ret.y += size.y;

    if (norm.z < 0)
        ret.z += size.z;

    return ret;
}

void Frustum::Plane::setPoints(const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &v3)
{
    m_normal = glm::normalize(glm::cross(v3 - v2, v1 - v2));
    m_point = v2;
    m_d = -glm::dot(m_normal, m_point);
}

void Frustum::Plane::set(const glm::vec3 &norm, const glm::vec3 &p)
{
    m_normal = glm::normalize(norm);
    m_d = -glm::dot(m_normal, m_point);
}

float Frustum::Plane::distance(const glm::vec3 &p)
{
    return m_d + glm::dot(m_normal, p);
}