// PS : pour run le code entrez dans votre terminal : "g++ -std=c++17 -c Main.cpp Card.cpp CardFactory.cpp Deck.cpp DiscardPile.cpp Hand.cpp Player.cpp Table.cpp TradeArea.cpp"
// Après compilation dans votre terminal entrez : "g++ -std=c++17 -o projet Main.o Card.o CardFactory.o Deck.o DiscardPile.o Hand.o Player.o Table.o TradeArea.o"
// Enfin entrez "./projet"


#include "CardFactory.h"
#include "Table.h"

int main() {
    std::string name1, name2;
    // On demande à l'utilisateur d'entrer les noms des deux joueurs.
    std::cout << "Enter Player 1 name: ";
    std::cin >> name1;
    std::cout << "Enter Player 2 name: ";
    std::cin >> name2;

    // On initialise la fabrique de cartes et le deck.
    CardFactory* factory = CardFactory::getFactory();
    Deck deck = factory->getDeck();
    Table table(name1, name2);

    // On distribue 5 cartes à chaque joueur pour commencer le jeu.
    for (int i = 0; i < 5; ++i) {
        if (!deck.empty()) table.getPlayer1().getHand() += deck.draw();
        if (!deck.empty()) table.getPlayer2().getHand() += deck.draw();
    }

    std::string winnerName;
    while (!deck.empty()) {
        for (int turn = 0; turn < 2; ++turn) {
            // On alterne les tours entre les deux joueurs.
            Player& currentPlayer = (turn == 0) ? table.getPlayer1() : table.getPlayer2();
            std::cout << "\n" << currentPlayer.getName() << "'s turn:\n";

            // On gère les cartes dans la Trade Area.
            if (table.getTradeArea().numCards() > 0) {
                std::cout << "Trade Area contains:\n";
                table.getTradeArea().print(std::cout);
                std::cout << "\nAdd cards to your chains or discard them? (add/discard): ";
                std::string choice;
                std::cin >> choice;

                // On ajoute ou jette les cartes selon le choix du joueur.
                if (choice == "add") {
                    while (table.getTradeArea().numCards() > 0) {
                        Card* tradeCard = table.getTradeArea().trade("");
                        try {
                            currentPlayer.getHand() += tradeCard;
                        } catch (...) {
                            table.getDiscardPile() += tradeCard;
                        }
                    }
                } else {
                    while (table.getTradeArea().numCards() > 0) {
                        table.getDiscardPile() += table.getTradeArea().trade("");
                    }
                }
            }

            // On joue la première carte de la main du joueur.
            if (!currentPlayer.getHand().empty()) {
                Card* playedCard = currentPlayer.getHand().play();
                std::cout << "Played card: " << *playedCard << "\n";

                bool addedToChain = false;
                for (int i = 0; i < currentPlayer.getNumChains(); ++i) {
                    if (currentPlayer[i].getTopCard()->getName() == playedCard->getName()) {
                        currentPlayer[i] += playedCard;
                        addedToChain = true;
                        break;
                    }
                }

                // On crée une nouvelle chaîne si nécessaire.
                if (!addedToChain) {
                    if (currentPlayer.getNumChains() < currentPlayer.getMaxNumChains()) {
                        currentPlayer.addChain(new Chain<Card>());
                        currentPlayer[currentPlayer.getNumChains() - 1] += playedCard;
                    } else {
                        std::cout << "You must sell a chain to make room for a new one.\n";
                        int chainToSell;
                        do {
                            std::cout << "Select a chain to sell (0-" << currentPlayer.getNumChains() - 1 << "): ";
                            std::cin >> chainToSell;
                        } while (chainToSell < 0 || chainToSell >= currentPlayer.getNumChains());

                        try {
                            // On vend une chaîne pour faire de la place.
                            int coinsEarned = currentPlayer[chainToSell].sell();
                            currentPlayer += coinsEarned;
                            std::cout << "You sold a chain for " << coinsEarned << " coins.\n";

                            currentPlayer.removeChain(chainToSell);
                            currentPlayer.addChain(new Chain<Card>());
                            currentPlayer[currentPlayer.getNumChains() - 1] += playedCard;
                        } catch (std::exception& e) {
                            std::cerr << "An error occurred: " << e.what() << "\n";
                        }
                    }
                }
            }

            // On pioche deux cartes pour le joueur.
            if (!deck.empty()) currentPlayer.getHand() += deck.draw();
            if (!deck.empty()) currentPlayer.getHand() += deck.draw();

            // On affiche l'état actuel de la table.
            std::cout << "\nUpdated table state:\n" << table << "\n";
        }
    }

    // On vérifie le gagnant lorsque le deck est vide.
    if (table.win(winnerName)) {
        std::cout << "\nThe game is over! The winner is: " << winnerName << "\n";
    } else {
        std::cout << "\nThe game is a draw!\n";
    }

    return 0;
}
