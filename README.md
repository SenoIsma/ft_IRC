# ft_irc - Fonctionnalités Implémentées

## 🔢 Commandes IRC prises en charge

### ✨ Authentification
- `PASS <password>` : vérifie le mot de passe
- `NICK <nickname>` : enregistre le pseudo du client
- `USER <username> 0 * :<realname>` : enregistre les infos utilisateur
- `QUIT [:reaon]` : Quitte le serveur

### ✨ Communication
- `PRIVMSG <target> :<message>` : message privé à un user ou un salon
- `NOTICE <target> :<message>` : message silencieux, sans erreur de retour

### ✨ Salon
- `JOIN <#channel>` : rejoindre ou créer un salon
- `PART <#channel>` : quitter un salon
- `INVITE <nick> <#channel>` : inviter un user à un salon
- `KICK <#channel> <nick> [:reason]` : expulser un user d'un salon
- `TOPIC <#channel> [:sujet]` : voir ou changer le sujet du salon
- `MODE <#channel>` : voir les modes actifs d'un salon
- `MODE <#channel> <mode> [param]` : changer les modes du salon

### ✨ Modes de salon gérés
| Mode | Description |
|------|-------------|
| `+t` | Seuls les opérateurs peuvent changer le topic |
| `+i` | Salon invite-only |
| `+k` | Salon avec mot de passe |
| `+l` | Limite du nombre de membres |
| `+o` | Ajout ou retrait du statut opérateur |
