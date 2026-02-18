# Analyse et Roadmap : Transformation ft_ping → ft_traceroute

## 📋 Résumé Exécutif

Ce document présente l'analyse des modifications nécessaires pour transformer votre projet `ft_ping` fonctionnel en `ft_traceroute` conforme au sujet fourni.

---

## 🔍 Justifications des Modifications

### 1. **Différences Conceptuelles Fondamentales**

#### Ping vs Traceroute
- **Ping** : Envoie des paquets vers une destination avec un TTL fixe et attend des réponses ECHO_REPLY
- **Traceroute** : Envoie des paquets avec un TTL incrémental (1, 2, 3...) et attend des réponses TIME_EXCEEDED de chaque routeur intermédiaire

#### Principe de Traceroute
1. Envoie un paquet avec TTL=1 → Le premier routeur répond avec ICMP_TIME_EXCEEDED
2. Envoie un paquet avec TTL=2 → Le deuxième routeur répond avec ICMP_TIME_EXCEEDED
3. Continue jusqu'à ce que le paquet atteigne la destination finale (ICMP_ECHOREPLY ou ICMP_DEST_UNREACH)

### 2. **Exigences du Sujet**

- **Nom de l'exécutable** : `ft_traceroute` (actuellement `ft_ping`)
- **Options** : Uniquement `--help` (actuellement : -v, -q, -c, -w, -ttl, -s)
- **FQDN** : Gérer le FQDN sans exécuter la résolution DNS dans l'affichage des sauts
- **Tolérance** : +/- 30ms sur un saut
- **Indentation** : Même indentation que le vrai traceroute
- **Fonctions interdites** : fcntl, poll, ppoll (déjà respecté)

---

## 🗺️ Roadmap Détaillée des Modifications

### **PHASE 1 : Restructuration de Base**

#### 1.1 Renommage et Structure
**Fichiers à modifier :**
- `Makefile` : Changer `NAME := ft_ping` → `NAME := ft_traceroute`
- `include/ft_ping.h` : Renommer en `ft_traceroute.h` et mettre à jour les gardes
- Tous les fichiers source : Mettre à jour les includes

**Justification** : Le sujet exige un exécutable nommé `ft_traceroute`. ======================================= OK

---

#### 1.2 Structure de Données (`ft_traceroute.h`)

**Modifications dans `t_options` :**
```c
// SUPPRIMER :
- int verbose;
- int quiet;
- int count;
- int deadline;
- int packet_size;

// CONSERVER :
- int help;

// AJOUTER :
- int max_hops;        // Nombre maximum de sauts (défaut: 30)
- int probes_per_hop;  // Nombre de sondes par saut (défaut: 3)
```

**Modifications dans `t_stats` :**
```c
// SUPPRIMER :
- double min_rtt;
- double max_rtt;
- double total_rtt;
- double total_rtt_squared;

// CONSERVER :
- unsigned int packets_send;
- unsigned int packets_received;
- unsigned int errors;

// AJOUTER :
- int current_hop;     // Saut actuel (TTL)
```

**Nouvelle structure `t_hop` :**
```c
typedef struct s_hop {
    struct sockaddr_in router_addr;  // Adresse du routeur
    char *hostname;                  // FQDN (si disponible)
    double rtt[3];                   // RTT des 3 probes
    int received_count;              // Nombre de réponses reçues
    int is_destination;              // 1 si c'est la destination finale
} t_hop;
```

**Modifications dans `t_ping` → `t_traceroute` :**
```c
typedef struct s_traceroute {
    t_options options;
    t_stats stats;
    char *target_host;
    struct sockaddr_in target_addr;
    int socket_fd;
    uint16_t sequence;
    pid_t pid;
    t_hop *hops;           // Tableau des sauts
    int max_hops;          // Taille du tableau
} t_traceroute;
```

**Justification** : Traceroute nécessite de stocker les informations de chaque saut (routeur intermédiaire), pas seulement des statistiques globales.

---

### **PHASE 2 : Parsing et Arguments**

#### 2.1 Simplification du Parsing (`parsing.c`)

**Modifications :**
- Supprimer la gestion de : `-v`, `-q`, `-c`, `-w`, `-s`, `-ttl`
- Conserver uniquement : `--help`
- Valider qu'un seul argument (adresse IPv4 ou nom d'hôte) est fourni

**Justification** : Le sujet exige uniquement l'option `--help`.

---================================ OK

### **PHASE 3 : Logique de Traceroute**

#### 3.1 Modification de la Boucle Principale (`main.c`)

**Changements majeurs :**

**AVANT (Ping) :**
```c
while (1) {
    send_packet(&ping);
    receive_packet(&ping, &send_time, &rtt, &ttl);
    sleep(1);
}
```

**APRÈS (Traceroute) :**
```c
for (int ttl = 1; ttl <= max_hops; ttl++) {
    for (int probe = 0; probe < 3; probe++) {
        send_packet_with_ttl(&traceroute, ttl);
        receive_packet(&traceroute, ttl, &rtt, &router_addr);
    }
    display_hop(&traceroute, ttl);
    if (destination_reached) break;
}
```

**Justification** : Traceroute incrémente le TTL et envoie 3 probes par saut par défaut.

---

#### 3.2 Modification de `send.c`

**Changements :**
- Ajouter un paramètre `ttl` à `send_packet()`
- Utiliser `setsockopt()` avec `IP_TTL` AVANT chaque envoi
- Ne plus utiliser un TTL fixe

**Nouvelle signature :**
```c
int send_packet_with_ttl(t_traceroute *traceroute, int ttl);
```

**Justification** : Chaque paquet doit être envoyé avec un TTL différent pour atteindre différents routeurs.

---

#### 3.3 Modification de `receive.c`

**Changements majeurs :**

1. **Gestion des réponses ICMP_TIME_EXCEEDED** :
   - Extraire l'adresse IP du routeur qui a envoyé TIME_EXCEEDED
   - Calculer le RTT
   - Stocker dans la structure `t_hop` correspondante

2. **Gestion de la destination finale** :
   - Détecter ICMP_ECHOREPLY (destination atteinte)
   - Détecter ICMP_DEST_UNREACH (destination inaccessible)
   - Marquer le saut comme destination finale

3. **Timeout** :
   - Si aucune réponse après un délai (ex: 5 secondes), afficher `*`

**Nouvelle signature :**
```c
int receive_packet_for_hop(t_traceroute *traceroute, int ttl, 
                           double *rtt, struct sockaddr_in *router_addr);
```

**Justification** : Traceroute doit distinguer les réponses TIME_EXCEEDED (routeurs intermédiaires) des réponses ECHO_REPLY (destination).

---

#### 3.4 Modification de `socket.c`

**Changements :**
- Supprimer la configuration du TTL fixe dans `create_socket()`
- Le TTL sera configuré dynamiquement dans `send.c`

**Justification** : Le TTL doit être modifié à chaque saut, pas une seule fois à la création.

---

### **PHASE 4 : Affichage**

#### 4.1 Nouveau Module `display.c`

**Remplacement complet de l'affichage :**

**Format attendu (comme le vrai traceroute) :**
```
 1  router1.example.com (192.168.1.1)  10.234 ms  10.123 ms  10.456 ms
 2  192.168.2.1                         20.123 ms  20.234 ms  *
 3  destination.com (192.168.3.1)       30.123 ms  30.234 ms  30.345 ms
```

**Fonctions à créer :**
```c
void display_hop(t_traceroute *traceroute, int hop_number);
void display_hop_line(t_hop *hop, int hop_number);
char *get_hostname_from_addr(struct sockaddr_in *addr);  // Sans résolution DNS
```

**Points importants :**
- Afficher le numéro de saut (1, 2, 3...)
- Afficher le FQDN si disponible (sans résolution DNS dans l'affichage)
- Afficher l'adresse IP entre parenthèses
- Afficher les 3 RTT (ou `*` si timeout)
- Tolérance de +/- 30ms (déjà gérée par l'affichage)

**Justification** : L'affichage de traceroute est complètement différent de ping (sauts au lieu de séquences).

---

#### 4.2 Gestion FQDN (`dns.c`)

**Modifications :**
- Conserver `resolve_hostname()` pour la résolution initiale de la destination
- **NE PAS** résoudre le DNS pour chaque routeur dans l'affichage
- Utiliser `getnameinfo()` ou `gethostbyaddr()` pour obtenir le FQDN des routeurs, mais seulement si disponible en cache

**Justification** : Le sujet exige "gérer le FQDN sans exécuter la résolution DNS dans l'affichage des sauts". Cela signifie probablement utiliser les informations déjà disponibles ou ne pas forcer de résolution.

---

### **PHASE 5 : Gestion des Timeouts et Erreurs**

#### 5.1 Timeout par Saut

**Modifications dans `receive.c` :**
- Utiliser `select()` avec un timeout (5 secondes par défaut)
- Si timeout, retourner une valeur spéciale indiquant l'absence de réponse
- Afficher `*` dans l'affichage pour les probes sans réponse

**Justification** : Traceroute doit gérer les routeurs qui ne répondent pas.

---

#### 5.2 Détection de la Destination

**Logique à implémenter :**
```c
if (icmp_reply->type == ICMP_ECHOREPLY) {
    // Destination atteinte
    mark_as_destination(hop);
    return DESTINATION_REACHED;
} else if (icmp_reply->type == ICMP_DEST_UNREACH) {
    // Destination inaccessible
    mark_as_destination(hop);
    return DESTINATION_UNREACHABLE;
} else if (icmp_reply->type == ICMP_TIME_EXCEEDED) {
    // Routeur intermédiaire
    return INTERMEDIATE_ROUTER;
}
```

**Justification** : Traceroute doit s'arrêter quand la destination est atteinte.

---

### **PHASE 6 : Statistiques et Nettoyage**

#### 6.1 Suppression des Statistiques Ping (`stats.c`)

**Modifications :**
- Supprimer `update_rtt_stats()` (calcul min/max/avg/stddev)
- Simplifier `init_stats()` pour traceroute
- Supprimer l'affichage des statistiques finales (ping statistics)

**Justification** : Traceroute n'affiche pas de statistiques globales comme ping.

---

#### 6.2 Gestion de la Mémoire

**Nouveau :**
- Allouer dynamiquement le tableau `hops` dans `t_traceroute`
- Libérer la mémoire à la fin du programme
- Libérer les `hostname` dans chaque `t_hop` si alloués

**Justification** : Le nombre de sauts est variable et inconnu à l'avance.

---

## 📝 Checklist de Migration

### Fichiers à Modifier
- [ ] `Makefile` : Renommer l'exécutable
- [ ] `include/ft_ping.h` → `include/ft_traceroute.h` : Restructurer les types
- [ ] `src/main.c` : Réécrire la boucle principale
- [ ] `src/parsing.c` : Simplifier pour --help uniquement
- [ ] `src/send.c` : Ajouter gestion TTL dynamique
- [ ] `src/receive.c` : Gérer TIME_EXCEEDED et destination
- [ ] `src/display.c` : Réécrire complètement l'affichage
- [ ] `src/socket.c` : Supprimer TTL fixe
- [ ] `src/dns.c` : Adapter pour FQDN sans résolution forcée
- [ ] `src/stats.c` : Simplifier ou supprimer
- [ ] `src/utils.c` : Conserver (checksum toujours nécessaire)
- [ ] `src/icmp.c` : Conserver (construction paquet ICMP)

### Fonctionnalités à Implémenter
- [ ] Boucle avec TTL incrémental (1 à max_hops)
- [ ] 3 probes par saut
- [ ] Gestion ICMP_TIME_EXCEEDED
- [ ] Détection destination (ECHOREPLY ou DEST_UNREACH)
- [ ] Timeout par probe (5 secondes)
- [ ] Affichage format traceroute (indentation correcte)
- [ ] Gestion FQDN sans résolution DNS forcée
- [ ] Option --help uniquement

### Tests à Effectuer
- [ ] Test avec adresse IP simple
- [ ] Test avec nom d'hôte (FQDN)
- [ ] Test avec destination locale
- [ ] Test avec destination distante
- [ ] Test avec routeurs qui ne répondent pas (*)
- [ ] Test avec destination inaccessible
- [ ] Vérification de l'indentation (identique au vrai traceroute)
- [ ] Vérification tolérance +/- 30ms

---

## 🎯 Points d'Attention Critiques

1. **TTL Dynamique** : Le TTL doit être modifié AVANT chaque envoi, pas une seule fois
2. **3 Probes par Saut** : Par défaut, traceroute envoie 3 paquets par saut
3. **Timeout** : Utiliser `select()` pour gérer les timeouts (fonction autorisée)
4. **FQDN** : Ne pas forcer de résolution DNS pour chaque routeur dans l'affichage
5. **Indentation** : L'affichage doit correspondre exactement au vrai traceroute
6. **Arrêt** : Le programme s'arrête quand la destination est atteinte ou après max_hops

---

## 📚 Références

- `man traceroute` : Documentation officielle
- Format d'affichage : Comparer avec la sortie de `traceroute` système
- ICMP : Comprendre les types ICMP_TIME_EXCEEDED, ICMP_ECHOREPLY, ICMP_DEST_UNREACH

---

## ⏱️ Estimation de Complexité

- **Phase 1 (Restructuration)** : ~2-3 heures
- **Phase 2 (Parsing)** : ~1 heure
- **Phase 3 (Logique)** : ~4-6 heures (partie la plus complexe)
- **Phase 4 (Affichage)** : ~2-3 heures
- **Phase 5 (Timeouts)** : ~2 heures
- **Phase 6 (Nettoyage)** : ~1 heure

**Total estimé** : ~12-16 heures de développement

---

*Document généré le : $(date)*
