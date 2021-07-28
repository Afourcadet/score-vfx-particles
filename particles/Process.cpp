#include "Process.hpp"

#include <Gfx/Graph/Node.hpp>
#include <Gfx/Graph/ShaderCache.hpp>
#include <Gfx/TexturePort.hpp>
#include <Process/Dataflow/Port.hpp>
#include <Process/Dataflow/WidgetInlets.hpp>

#include <wobjectimpl.h>
W_OBJECT_IMPL(particles::Model)
namespace particles
{
    Model::Model(
                const TimeVal& duration,
                const Id<Process::ProcessModel>& id,
                QObject* parent)
        : Process::ProcessModel{duration, id, "gfxProcess", parent}
    {
        metadata().setInstanceName(*this);
        {
            auto pointSize = new Process::IntSlider{Id<Process::Port>(2), this};
            pointSize->setName(tr("Point size"));
            pointSize->setValue(28.);
            pointSize->setDomain(ossia::make_domain(1.f, 300.f));
            m_inlets.push_back(pointSize);
        }
        m_outlets.push_back(new Gfx::TextureOutlet{Id<Process::Port>(0), this});
    }

    Model::~Model() { }

    QString Model::prettyName() const noexcept
    {
        return tr("My VFX");
    }

}
template <>
void DataStreamReader::read(const particles::Model& proc)
{
    readPorts(*this, proc.m_inlets, proc.m_outlets);

    insertDelimiter();
}

template <>
void DataStreamWriter::write(particles::Model& proc)
{
    writePorts(
                *this,
                components.interfaces<Process::PortFactoryList>(),
                proc.m_inlets,
                proc.m_outlets,
                &proc);
    checkDelimiter();
}

template <>
void JSONReader::read(const particles::Model& proc)
{
    readPorts(*this, proc.m_inlets, proc.m_outlets);
}

template <>
void JSONWriter::write(particles::Model& proc)
{
    writePorts(
                *this,
                components.interfaces<Process::PortFactoryList>(),
                proc.m_inlets,
                proc.m_outlets,
                &proc);
}
