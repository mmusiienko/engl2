#pragma once

#include "../core/Core.h"
#include "../math/Math.h"
#include <vector>
#include <tuple>
#include <queue>
#include <unordered_map>
#include <bitset>
#include <any>

namespace EnGl
{
    template<typename T>
    concept ECSComponent =
        std::is_default_constructible_v<T> &&
        std::is_copy_constructible_v<T> &&
        std::is_copy_assignable_v<T> &&
        std::is_move_constructible_v<T> &&
        std::is_move_assignable_v<T> &&
        std::is_trivially_destructible_v<T>;

    template<ECSComponent... AllCs>
    struct Ecs
    {
        using Entity = u32;
        static constexpr size_t NComponents = sizeof...(AllCs);
        using CSignature = std::bitset<NComponents>;

        class ComponentStorage
        {
        public:
            struct CreationInfo
            {
                u32 ArchetypeId;
            };

            template<ECSComponent... Cs>
            CreationInfo Create(Entity id)
            {
                ArchetypeInfo& arch = GetArchetypeStorage<Cs...>();

                if (arch.Size < arch.Entities.size())
                {
                    ((std::get<std::vector<Cs>>(arch.Components)[arch.Size] = Cs{}), ...);
                    arch.Entities[arch.Size] = id;
                }
                else
                {
                    ((std::get<std::vector<Cs>>(arch.Components).push_back(Cs{})), ...);
                    arch.Entities.push_back(id);
                }
                arch.GlobalToLocal.insert({ id, arch.Size });
                arch.Size++;

                return { .ArchetypeId = arch.Id };
            }

            template<ECSComponent... Cs, typename F>
            CreationInfo Create(Entity id, F&& initializer)
            {
                ArchetypeInfo& arch = GetArchetypeStorage<Cs...>();
                std::tuple<Cs...> components{};
                std::apply(std::forward<F>(initializer), components);

                if (arch.Size < arch.Entities.size())
                {
                    ((std::get<std::vector<Cs>>(arch.Components)[arch.Size] = std::get<Cs>(components)), ...);
                    arch.Entities[arch.Size] = id;
                }
                else
                {
                    ((std::get<std::vector<Cs>>(arch.Components).push_back(std::get<Cs>(components))), ...);
                    arch.Entities.push_back(id);
                }
                
                arch.GlobalToLocal.insert({ id, arch.Size });
                arch.Size++;

                return { .ArchetypeId = arch.Id };
            }

            template<ECSComponent C1, ECSComponent C2, ECSComponent... Cs>
            std::tuple<C1&, C2&, Cs&...> Get(Entity entity, u32 archetypeId)
            {
                ArchetypeInfo* arch = m_Archetypes[archetypeId];
                
                Entity localId = arch->GlobalToLocal.at(entity);

                return std::tie(
                    std::get<std::vector<C1>>(arch->Components)[localId],
                    std::get<std::vector<C2>>(arch->Components)[localId],
                    std::get<std::vector<Cs>>(arch->Components)[localId]... 
                );
            }

            template<ECSComponent C>
            C& Get(Entity entity, u32 archetypeId)
            {
                ArchetypeInfo* arch = m_Archetypes[archetypeId];

                Entity localId = arch->GlobalToLocal.at(entity);

                return std::get<std::vector<C>>(arch->Components)[localId];
            }

            template<ECSComponent Cs>
            bool Has(Entity entity, u32 archetypeId)
            {
                ArchetypeInfo* arch = m_Archetypes[archetypeId];

                return arch->Signature[GetComponentId<Cs>()] && arch->GlobalToLocal.contains(entity);
            }

            void Remove(u32 archetypeId, Entity id)
            {
                ArchetypeInfo& arch = *m_Archetypes[archetypeId];

                Entity localId = arch.GlobalToLocal.at(id);
                Entity back = arch.Size - 1;
                Entity moved = arch.Entities[back];

                arch.GlobalToLocal.erase(id);
                if (localId != back)
                {
                    std::swap(arch.Entities[localId], arch.Entities[back]);

                    std::apply([localId, back](auto&... vectors) -> void
                        {
                            ((vectors.empty() ? void() : std::swap(vectors[localId], vectors[back])), ...);
                        }
                    , arch.Components);

                    arch.GlobalToLocal.erase(moved);
                    arch.GlobalToLocal.insert({ moved, localId });
                }
                
                arch.Size--;
            }

            template<ECSComponent... Cs>
            size_t CountAll()
            {
                size_t count = 0;

                CSignature signature = GetSignature<Cs...>();

                for (const auto arch : m_Archetypes)
                {
                    if ((arch->Signature & signature) == 0) continue;

                    (( count += std::get<std::vector<Cs>&>(arch->Components).size() ), ...);
                }

                return count;
            }

            template<ECSComponent C>
            u32 GetComponentId()
            {
                static u32 id = m_TotalComponents++;
                return id;
            }

            template<ECSComponent... Cs>
            CSignature& GetSignature()
            {
                static CSignature signature;
                static bool first = (SetSignature<Cs...>(signature), true);
                return signature;
            }

            template<ECSComponent... Cs>
            void SetSignature(CSignature& signature)
            {
                ((signature[GetComponentId<Cs>()] = true), ...);
            }

        private:
            struct ArchetypeInfo
            {
                CSignature Signature;
                size_t Size = 0;
                std::vector<Entity> Entities;
                std::tuple<std::vector<AllCs>...> Components;
                std::unordered_map<Entity, Entity> GlobalToLocal;
                u32 Id = 0;
                ArchetypeInfo(CSignature signature) : Signature(std::move(signature)) {}
            };

            void OnArchRegister(ArchetypeInfo* info)
            {
                info->Id = static_cast<Entity>(m_Archetypes.size());
                m_Archetypes.push_back(info);
;            }

            template<ECSComponent... Cs>
            ArchetypeInfo& GetArchetypeStorage()
            {
                static ArchetypeInfo info{ GetSignature<Cs...>() };
                static bool onRegister = (OnArchRegister(&info), true);
                return info;
            };

            template<ECSComponent... Cs>
            struct ArchIterator
            {
                ArchIterator(ComponentStorage* storage, ArchetypeInfo* arch, size_t curr)
                    : storage(storage), arch(arch), curr(curr) {
                }

                ArchIterator() = default;

                ArchIterator<Cs...>& operator++()
                {
                    curr++;
                    return *this;
                }

                ArchIterator<Cs...> operator++(int)
                {
                    auto copy = *this;
                    ++(*this);
                    return copy;
                }

                bool operator!=(const ArchIterator& other) const { return curr != other.curr; }
                bool operator==(const ArchIterator& other) const { return curr == other.curr; }

                auto operator*()
                {
                    return std::tie(
                        arch->Entities[curr],
                        std::get<std::vector<Cs>>(arch->Components)[curr]...
                    );
                }

            private:
                ComponentStorage* storage = nullptr;
                ArchetypeInfo* arch = nullptr;
                size_t curr = 0;
            };

            template<ECSComponent... Cs>
            struct ArchIteratorStruct
            {
                ArchIteratorStruct(ComponentStorage* storage, ArchetypeInfo* arch)
                    : storage(storage), arch(arch) {
                }

                ArchIteratorStruct() = default;

                ArchIterator<Cs...> begin() { return ArchIterator<Cs...>{storage, arch, 0}; }
                ArchIterator<Cs...> end() { return ArchIterator<Cs...>{storage, arch, arch->Size}; }

            private:
                ComponentStorage* storage = nullptr;
                ArchetypeInfo* arch = nullptr;
            };

            template<ECSComponent C>
            std::queue<Entity>& GetDeadComponentStorage() const
            {
                static std::queue<Entity> storage;
                return storage;
            };

            std::vector<ArchetypeInfo*> m_Archetypes;
            u32 m_TotalComponents = 0;

        public:
            template<ECSComponent... Cs>
            struct QueryIterator
            {
                QueryIterator(ComponentStorage* storage, CSignature signature, CSignature excludeSignature, size_t c = 0)
                    : storage(storage), signature(std::move(signature)), excludeSignature(std::move(excludeSignature))
                {
                    if (c < storage->m_Archetypes.size())
                    {
                        iCurr = c;
                        SkipInvalid();
                    }
                    else
                    {
                        iCurr = storage->m_Archetypes.size();
                    }
                }

                template<ECSComponent... CEs>
                QueryIterator<Cs...>& Without()
                {
                    excludeSignature = storage->GetSignature<CEs...>();
                    return *this;
                }

                QueryIterator<Cs...>& operator++()
                {
                    ++curr;
                    if (curr == end)
                    {
                        ++iCurr;
                        SkipInvalid();
                    }
                    return *this;
                }

                QueryIterator<Cs...> operator++(int)
                {
                    auto copy = *this;
                    ++(*this);
                    return copy;
                }

                bool operator!=(const QueryIterator& other) const { return iCurr != other.iCurr; }
                bool operator==(const QueryIterator& other) const { return iCurr == other.iCurr; }

                auto operator*() { return *curr; }

            private:
                void SkipInvalid()
                {
                    while (iCurr < storage->m_Archetypes.size())
                    {
                        auto arch = storage->m_Archetypes[iCurr];
                        bool matches = ((arch->Signature & signature) == signature);
                        bool dontExclude = (arch->Signature & excludeSignature).none();

                        if (arch->Size == 0 || !matches || !dontExclude)
                        {
                            ++iCurr;
                        }
                        else
                        {
                            currIter = ArchIteratorStruct<Cs...>{ storage, arch };
                            curr = currIter.begin();
                            end = currIter.end();
                            break;
                        }
                    }
                }

                CSignature signature;
                CSignature excludeSignature;
                ComponentStorage* storage = nullptr;
                size_t iCurr;
                ArchIterator<Cs...> curr;
                ArchIterator<Cs...> end;
                ArchIteratorStruct<Cs...> currIter;
            };

            template<ECSComponent... Cs>
            struct QueryStruct
            {
                QueryStruct(ComponentStorage* storage)
                    : storage(storage), signature(storage->GetSignature<Cs...>()) {
                }

                QueryIterator<Cs...> begin()
                {
                    return QueryIterator<Cs...>{storage, signature, excludeSignature, 0};
                }

                QueryIterator<Cs...> end()
                {
                    return QueryIterator<Cs...>{storage, signature, excludeSignature, storage->m_Archetypes.size()};
                }

                template<ECSComponent... CsE>
                QueryStruct<Cs...>& Exclude()
                {
                    excludeSignature = storage->GetSignature<CsE...>();
                    return *this;
                }

            private:
                Ecs<AllCs...>::ComponentStorage* storage;
                CSignature signature;
                CSignature excludeSignature;
            };

            template<ECSComponent... Cs>
            QueryStruct<Cs...> Query() { return QueryStruct<Cs...>{this}; }
        };

        class EntityManager
        {
        public:
            template<ECSComponent... Cs>
            Entity Create()
            {
                Entity id = NextId();
                auto res = m_ComponentStorage.Create<Cs...>(id);

                (id == m_Entities.size()) ? NewEntity(res) : ReuseEntity(res, id);
                
                return id;
            }

            template<ECSComponent... Cs, typename F>
            Entity Create(F&& initializer)
            {
                Entity id = NextId();
                auto res = m_ComponentStorage.Create<Cs...>(id, std::forward<F>(initializer));

                (id == m_Entities.size()) ? NewEntity(res) : ReuseEntity(res, id);

                return id;
            }

            template<ECSComponent C1, ECSComponent C2, ECSComponent... Cs>
            std::tuple<C1&, C2&, Cs&...> Get(Entity id)
            {
                return m_ComponentStorage.Get<C1, C2, Cs...>(id, m_ArchIds[id]);
            }

            template<ECSComponent C>
            C& Get(Entity id)
            {
                return m_ComponentStorage.Get<C>(id, m_ArchIds[id]);
            }

            template<ECSComponent Cs>
            bool Has(Entity id)
            {
                return m_ComponentStorage.Has<Cs>(id, m_ArchIds[id]);
            }

            void Remove(Entity id)
            {
                if (!m_Entities[id]) return;

                m_ComponentStorage.Remove(m_ArchIds[id], id);
                m_DeadIds.push(id);
                m_Entities[id] = false;
            }

            template<ECSComponent... Cs>
            u32 CountAll()
            {
                return m_ComponentStorage.CountAll<Cs...>();
            }

            template<ECSComponent... Cs>
            Ecs<AllCs...>::ComponentStorage::QueryStruct<Cs...> Query()
            {
                return m_ComponentStorage.Query<Cs...>();
            }

        private:
            void NewEntity(Ecs<AllCs...>::ComponentStorage::CreationInfo& res)
            {
                m_Entities.push_back(true);
                m_ArchIds.push_back(res.ArchetypeId);
                m_Generations.push_back(0);
            }

            void ReuseEntity(Ecs<AllCs...>::ComponentStorage::CreationInfo& res, Entity id)
            {
                m_Entities[id] = true;
                m_ArchIds[id] = res.ArchetypeId;
                m_Generations[id]++;
            }

            Entity NextId()
            {
                if (!m_DeadIds.empty())
                {
                    Entity id = m_DeadIds.front();
                    m_DeadIds.pop();
                    return id;
                }

                return static_cast<Entity>(m_Entities.size());
            }

            std::queue<Entity> m_DeadIds;
            std::vector<bool> m_Entities;
            std::vector<u32> m_Generations;
            std::vector<u32> m_ArchIds;
            Ecs<AllCs...>::ComponentStorage m_ComponentStorage{};
        };

        template<typename Context>
        class System
        {
        public:
            virtual void Run(Ecs<AllCs...>::EntityManager& manager, Context& context) {};
            virtual void Init(Ecs<AllCs...>::EntityManager& manager) {};
            virtual void Editor(Ecs<AllCs...>::EntityManager& manager, Context& context) {};
            virtual ~System() = default;
        };

        template<typename Context>
        class SystemRegistry
        {
        public:
            SystemRegistry(Ecs<AllCs...>::EntityManager& manager) : m_Manager(manager) {}

            template<typename T, typename... Args>
                requires std::derived_from<T, typename Ecs<AllCs...>::template System<Context>>
            SystemRegistry& Add(Args&&... args)
            {
                m_Systems.push_back(make_scope<T>( std::forward<Args>(args)... ));
                return *this;
            }

            void Init()
            {
                for (auto& system : m_Systems)
                {
                    system->Init(m_Manager);
                }
            }

            void Editor(Context& context)
            {
                for (auto& system : m_Systems)
                {
                    system->Editor(m_Manager, context);
                }
            }

            void Run(Context& context)
            {
                u32 i = 0;
                for (auto& system : m_Systems)
                {
                    auto start = glfwGetTime();
                    system->Run(m_Manager, context);
                    i++;
                    auto end = glfwGetTime();
                    //spdlog::info("{} took {}", i, end - start);
                }
            }

        private:
            std::vector< scope< typename Ecs<AllCs...>::template System<Context> > > m_Systems;
            Ecs<AllCs...>::EntityManager& m_Manager;
        };

    private:
        Ecs<AllCs...>::EntityManager m_Entity{};
    public:
        template<typename Context>
        Ecs<AllCs...>::SystemRegistry<Context>& eSystem() { 
            static typename Ecs<AllCs...>::template SystemRegistry<Context> systemRegistry { m_Entity };
            return systemRegistry;
        }

        Ecs<AllCs...>::EntityManager& eEntity() { return m_Entity; }
    };
}
