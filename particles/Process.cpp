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
            auto speedModifier = new Process::FloatSlider{Id<Process::Port>(2), this};
            speedModifier->setName(tr("Speed Modifier"));
            speedModifier->setValue(28.);
            speedModifier->setDomain(ossia::make_domain(1.f, 300.f));
            m_inlets.push_back(speedModifier);
        }
        {
            auto nbrParticles = new Process::IntSlider{Id<Process::Port>(3), this};
            nbrParticles->setName(tr("Number Of Particles"));
            nbrParticles->setValue(1+maxparticles/2);
            nbrParticles->setDomain(ossia::make_domain(1, maxparticles));
            m_inlets.push_back(nbrParticles);
        }
        {
            auto meshnameinput = new Process::LineEdit{Id<Process::Port>(4), this};
            meshnameinput->setName(tr("Name of the mesh file"));
            meshnameinput->setValue("ponte.obj");
            m_inlets.push_back(meshnameinput);
        }
        m_outlets.push_back(new Gfx::TextureOutlet{Id<Process::Port>(0), this});
    }

    Model::~Model() { }

    QString Model::prettyName() const noexcept
    {
        return tr("Particles");
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
