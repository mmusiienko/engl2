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
                CSignature Signature;
            };

            template<ECSComponent... Cs>
            CreationInfo Create(Entity id)
            {
                auto signature = GetSignature<Cs...>();
                ArchetypeInfo& arch = GetArchetypeStorage(signature);

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

                return { .Signature = signature };
            }

            template<ECSComponent... Cs, typename F>
            CreationInfo Create(Entity id, F&& initializer)
            {
                auto signature = GetSignature<Cs...>();
                ArchetypeInfo& arch = GetArchetypeStorage(signature);

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

                return { .Signature = signature };
            }

            template<ECSComponent C1, ECSComponent C2, ECSComponent... Cs>
            std::tuple<C1&, C2&, Cs&...> Get(Entity entity, const CSignature& signature)
            {
                ArchetypeInfo& arch = m_Archetypes[signature];
                
                Entity localId = arch.GlobalToLocal.at(entity);

                return std::tie(
                    std::get<std::vector<C1>>(arch.Components)[localId],
                    std::get<std::vector<C2>>(arch.Components)[localId],
                    std::get<std::vector<Cs>>(arch.Components)[localId]...
                );
            }

            template<ECSComponent C>
            C& Get(Entity entity, const CSignature& signature)
            {
                ArchetypeInfo& arch = m_Archetypes[signature];

                Entity localId = arch.GlobalToLocal.at(entity);

                return std::get<std::vector<C>>(arch.Components)[localId];
            }

            template<ECSComponent Cs>
            bool Has(Entity entity, const CSignature& signature)
            {
                ArchetypeInfo& arch = m_Archetypes[signature];

                return signature[GetComponentId<Cs>()] && arch.GlobalToLocal.contains(entity);
            }

            void Remove(Entity id, const CSignature& signature)
            {
                ArchetypeInfo& arch = m_Archetypes[signature];

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

                for (const auto& [archSignature, arch] : m_Archetypes)
                {
                    if ((archSignature & signature) != signature) continue;

                    (( count += std::get<std::vector<Cs>&>(arch.Components).size() ), ...);
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
            CSignature GetSignature()
            {
                CSignature signature;
                ((signature[GetComponentId<Cs>()] = true), ...);
                return signature;
            }

        private:
            struct ArchetypeInfo
            {
                size_t Size = 0;
                std::vector<Entity> Entities;
                std::tuple<std::vector<AllCs>...> Components;
                std::unordered_map<Entity, Entity> GlobalToLocal;
                u32 Id = 0;
            };

            ArchetypeInfo& GetArchetypeStorage(const CSignature& signature)
            {
                return m_Archetypes[signature];
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

            std::unordered_map<CSignature, ArchetypeInfo> m_Archetypes;
            u32 m_TotalComponents = 0;

        public:
            template<ECSComponent... Cs>
            struct QueryIterator
            {
                using iter = std::unordered_map<CSignature, ArchetypeInfo>::iterator;

                QueryIterator(ComponentStorage* storage, CSignature signature, CSignature excludeSignature, iter c = 0)
                    : storage(storage), signature(std::move(signature)), excludeSignature(std::move(excludeSignature))
                {
                    iCurr = c;
                    SkipInvalid();
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
                    while (iCurr != storage->m_Archetypes.end())
                    {
                        auto arch = &(*iCurr).second;
                        auto archSignature = (*iCurr).first;
                        bool matches = ((archSignature & signature) == signature);
                        bool dontExclude = (archSignature & excludeSignature).none();

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
                iter iCurr;
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
                    return QueryIterator<Cs...>{storage, signature, excludeSignature, storage->m_Archetypes.begin()};
                }

                QueryIterator<Cs...> end()
                {
                    return QueryIterator<Cs...>{storage, signature, excludeSignature, storage->m_Archetypes.end()};
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
            Entity Create(std::string name = "Entity")
            {
                Entity id = NextId();
                auto res = m_ComponentStorage.Create<Cs...>(id);

                (id == m_Entities.size()) ? NewEntity(res, std::move(name)) : ReuseEntity(res, std::move(name), id);
                
                return id;
            }

            template<ECSComponent... Cs, typename F>
            Entity Create(F&& initializer, std::string name = "Entity")
            {
                Entity id = NextId();
                auto res = m_ComponentStorage.Create<Cs...>(id, std::forward<F>(initializer));

                (id == m_Entities.size()) ? NewEntity(res, std::move(name)) : ReuseEntity(res, std::move(name), id);

                return id;
            }

            template<ECSComponent C1, ECSComponent C2, ECSComponent... Cs>
            std::tuple<C1&, C2&, Cs&...> Get(Entity id)
            {
                return m_ComponentStorage.Get<C1, C2, Cs...>(id, m_Signatures[id]);
            }

            template<ECSComponent C>
            C& Get(Entity id)
            {
                return m_ComponentStorage.Get<C>(id, m_Signatures[id]);
            }

            const std::string& GetName(Entity id) const
            {
                return m_Names[id];
            }

            template<ECSComponent Cs>
            bool Has(Entity id)
            {
                return m_ComponentStorage.Has<Cs>(id, m_Signatures[id]);
            }

            void Remove(Entity id)
            {
                if (!m_Entities[id]) return;

                m_ComponentStorage.Remove(id, m_Signatures[id]);
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

            struct Iterator
            {
                Iterator(EntityManager* manager, const CSignature& signature, Entity c = 0) : Manager(manager), Signature(signature), Curr(c)
                {
                    SkipInvalid();
                }

                Iterator begin()
                {
                    return Iterator(Manager, Signature, 0);
                }

                Iterator end()
                {
                    return Iterator(Manager, Signature, Manager->m_Entities.size());
                }

                void SkipInvalid()
                {
                    while (
                        Curr < Manager->m_Entities.size() && !Manager->m_Entities[Curr] &&
                        ((Signature & Manager->m_Signatures[Curr]) == Signature)
                        )
                    {
                        Curr++;
                    }
                }

                Iterator& operator++()
                {
                    Curr++;
                    return *this;
                }

                Iterator operator++(int)
                {
                    auto copy = *this;
                    ++(*this);
                    return copy;
                }

                bool operator==(const Iterator& other) const
                {
                    return Curr == other.Curr && Signature == other.Signature;
                }

                bool operator!=(const Iterator& other) const
                {
                    return !(*this == other);
                }

                Entity operator*()
                {
                    return Curr;
                }

                const CSignature& Signature;
                EntityManager* Manager = nullptr;
            private:
                Entity Curr = 0;
            };

            Iterator Query(const CSignature& signature)
            {
                return Iterator{ this, signature };
            }

        private:
            void NewEntity(Ecs<AllCs...>::ComponentStorage::CreationInfo& res, std::string&& name)
            {
                m_Entities.push_back(true);
                m_Signatures.push_back(res.Signature);
                m_Names.push_back(std::move(name));
                m_Generations.push_back(0);
            }

            void ReuseEntity(Ecs<AllCs...>::ComponentStorage::CreationInfo& res, std::string&& name, Entity id)
            {
                m_Entities[id] = true;
                m_Signatures[id] = res.Signature;
                m_Names[id] = std::move(name);
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
            std::vector<bool> m_Entities{false};
            std::vector<std::string> m_Names{"default"};
            std::vector<u32> m_Generations{0};
            std::vector<CSignature> m_Signatures{0};
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
                m_Systems.push_back(make_scope<T>(std::forward<Args>(args)...));
                m_Names.push_back(typeid(T).name());
                return *this;
            }

            void Init()
            {
                for (auto& system : m_Systems)
                {
                    system->Init(m_Manager);
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

            const auto& Systems() const
            {
                return m_Systems;
            }

            auto& Get(size_t idx) const
            {
                return m_Systems[idx];
            }

            const std::string& GetName(size_t idx) const
            {
                return m_Names[idx];
            }

        private:
            std::vector< scope< typename Ecs<AllCs...>::template System<Context> > > m_Systems;
            std::vector< std::string > m_Names;
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
