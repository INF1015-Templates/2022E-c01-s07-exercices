///
/// Ce projet utilise quelques librairies disponibles sur Vcpkg et permet entre autre d'en tester le bon fonctionnement.
///


#include <cstddef>
#include <cstdint>

#include <array>
#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <vector>
#include <span>
#include <utility>

#include <cppitertools/itertools.hpp>
#include <cppitertools/zip.hpp>

#include "MyClass.hpp"
#include "School.hpp"
#include "Animal.hpp"

using namespace std;
using namespace iter;


void runSimpleVtableExample() {
	struct Spam {
		virtual ~Spam() = default;
		virtual void fn1() const { cout << "Spam::fn1()" << "\n"; }
		int fn2() const { return m1; }
		int m1 = 0x11111111;
	};

	struct Eggs : public Spam {
		void fn1() const override { cout << "Eggs::fn1()" << "\n"; }
		int fn2() const { return m1 + m2; }
		int m2 = 0x22222222;
	};

	struct Parrot : public Eggs {
		void fn1() const override { cout << "Parrot::fn1()" << "\n"; }
		int fn2() const { return m1 + m2 + m3; }
		int m3 = 0x33333333;
	};

	// En inspectant la mémoire, on voit que parrot a un pointeur de vtable au début, puis les données dans l'ordre d'héritage
	Parrot parrot;
	cout << sizeof(parrot) << "\n";

	// En faisant des conversions (up-casting), on voit que l'adresse ne change pas.
	cout << &parrot << "\n"
	     << static_cast<Eggs*>(&parrot) << "\n"
	     << static_cast<Spam*>(&parrot) << "\n";

	Spam& ref = parrot;
	// En inspectant l'assembleur, on voit que pour appeler fn1() (méthode virtuelle), il faut d'abord résoudre son adresse à travers le vtable.
	ref.fn1();
	// Ici, l'appel à fn2() (méthode non-virtuelle) est direct.
	int res = ref.fn2();
	cout << hex << res;
}

void runVirtualInheritanceVtableExample() {
	struct Spam {
		virtual ~Spam() = default;
		int m1 = 0x11111111;
	};

	struct Eggs : virtual public Spam {
		virtual ~Eggs() = default;
		int m2 = 0x22222222;
	};

	struct Parrot : virtual public Spam {
		virtual ~Parrot() = default;
		int m3 = 0x33333333;
	};

	struct ExParrot : public Eggs, public Parrot {
		virtual ~ExParrot() = default;
		int m4 = 0x44444444;
	};

	// En inspectant la mémoire, on voit que exParrot a trois pointeurs de vtable distincts.
	ExParrot exParrot;
	cout << sizeof(exParrot) << "\n";

	// En faisant les conversions, on voit que l'adresse change.
	cout << &exParrot << "\n"
	     << static_cast<Eggs*>(&exParrot) << "\n"
	     << static_cast<Parrot*>(&exParrot) << "\n"
	     << static_cast<Spam*>(&exParrot) << "\n";

	// En inspectant l'assembleur généré, on voit que l'accès à m2, m3 et m4 est direct.
	exParrot.m4 = 0x88888888;
	exParrot.m2 = 0x66666666;
	exParrot.m3 = 0x77777777;
	// Ici, on voit que pour savoir comment accéder à m1 (membre de la base virtuelle), il faut passer par le vtable.
	exParrot.m1 = 0x55555555;
}

void runSchoolExample() {
	// Exemple prof/étudiant (School.hpp)
	Professor nancySirois(0xDEADBEEF);
	GraduateStudent lukasLehoux(69420, "Maitrise en nenie fomatique", &nancySirois);
		
	Professor karlBachand(0xBADCAFE);
	ChaoticEvilStudent alexDesrosiers(0xFEEDBABE, "Genie chimique AKA 'comment optimiser la production de meth'", &karlBachand);
	Student* ptr = &alexDesrosiers;
	// 'ptr' est un pointeur de Etudiant (up-casté de MauvaisEtudiantBanditPasCorrect)
	// Si on le dynamic_cast en GradStudent*, ça marche puisque MauvaisEtudiantBanditPasCorrect est dérivé
	// de GradStudent, et donc en est un.
	if (dynamic_cast<GraduateStudent*>(ptr) != nullptr)
		cout << "Sup" << endl;
	else
		cout << "Pas sup" << endl;

	// Destructeur de MauvaisEtudiantBanditPasCorrect se fait appelé à la fin du scope : I'll be back!
}

void runAnimalExample() {
	Liger unholyAbomination(1337);
	unholyAbomination.live();
	Lion& simba = unholyAbomination;
	Tiger& tony = unholyAbomination;
	Animal& animal = unholyAbomination;
	simba.live();
	tony.live();
	animal.live();
}

void runAmbiguousCallExample() {
	class LegalEntity {
	public:
		LegalEntity(int id) : id_(id) { }

		int getId() const {
			return id_;
		}

	private:
		int id_;
	};

	class StockExchangeEntity {
	public:
		StockExchangeEntity(int id) : id_(id) { }

		int getId() const {
			return id_;
		}

	private:
		int id_;
	};

	class PublicCompany : public LegalEntity, public StockExchangeEntity {
	public:
		PublicCompany(int legalId, int xchgId)
		: LegalEntity(legalId),
		  StockExchangeEntity(xchgId) {
			
		}

		//using LegalEntity::getId;

		int getReq() const {
			return LegalEntity::getId();
		}

		int getStonksId() const {
			return StockExchangeEntity::getId();
		}

		pair<int, int> getId() const {
			return {LegalEntity::getId(), StockExchangeEntity::getId()};
		}
	};

	PublicCompany foo(0xDEAD, 0xBEEF);
	auto&& [req, xchg] = foo.getId();
	cout << hex << req << "\n";
	cout << hex << xchg << "\n";
	//cout << hex << foo.getId() << "\n";
	//cout << hex << foo.getReq() << "\n";
	//cout << hex << foo.getStonksId() << "\n";
}


int main() {
	//runAmbiguousCallExample(); cout << "\n\n\n\n";
	//runSimpleVtableExample(); cout << "\n\n\n\n";
	//runVirtualInheritanceVtableExample(); cout << "\n\n\n\n";
	//runSchoolExample(); cout << "\n\n\n\n";
	//runAnimalExample(); cout << "\n\n\n\n";
}

